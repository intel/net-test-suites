/*
 * Copyright © 2017-2018, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <sys/socket.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "PortQemu.hh"

namespace PortQemu__Module {

#define E(fmt, args...)							\
do {									\
	TTCN_error("%s:%s() Error: " fmt "(): %s",			\
		__FILE__, __func__, ## args, strerror(errno));		\
} while (0)

#define W(fmt, args...)							\
do {									\
	TTCN_warning("%s:%s() " fmt, __FILE__, __func__, ## args);	\
} while (0)

PortQemu::PortQemu(const char *port_name) : PortQemu_BASE(port_name)
{
	src_ip = inet_addr("127.0.0.1");
	dst_ip = inet_addr("127.0.0.1");

	src_port = 7777;
	dst_port = 7771;

	inet_pton(AF_INET6, "fe80::2", &src_ip6);
	inet_pton(AF_INET6, "0:0:0:0:0:0:0:2", &dst_ip6);
}

PortQemu::~PortQemu() { }

#define IS_PARAM(_param, _name) (strcmp(_param, _name) == 0)
#define _IS(_s1, _s2) (strcmp(_s1, _s2) == 0)

void PortQemu::set_parameter(const char *name, const char *value)
{
	if (IS_PARAM("src_eth", name)) {
		memcpy(&src_eth, ether_aton(value), sizeof(src_eth));
		goto out;
	}

	if (IS_PARAM("src_ip", name)) {
		dst_port = inet_addr(value);
		goto out;
	}

	if (IS_PARAM("src_ip6", name)) {
		inet_pton(AF_INET6, value, &src_ip6);
		goto out;
	}

	if (IS_PARAM("src_port", name)) {
		src_port = atoi(value);
		goto out;
	}

	if (IS_PARAM("dst_eth", name)) {
		memcpy(&dst_eth, ether_aton(value), sizeof(dst_eth));
		goto out;
	}

	if (IS_PARAM("dst_ip", name)) {
		dst_ip = inet_addr(value);
		goto out;
	}

	if (IS_PARAM("dst_ip6", name)) {
		inet_pton(AF_INET6, value, &dst_ip6);
		goto out;
	}

	if (IS_PARAM("dst_port", name)) {
		dst_port = atoi(value);
		goto out;
	}

	if (IS_PARAM("vlan_enabled", name) && _IS(value, "true")) {
		vlan_enabled = true;
		goto out;
	}

	W("Unsupported parameter: %s", name);
 out:
	return;
}

void PortQemu::Handle_Fd_Event_Readable(int fd)
{
	uint8_t *data = buf;
	ssize_t offset = sizeof(struct ip6_hdr) + sizeof(struct ethhdr);
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (data + sizeof(struct ethhdr));
	ssize_t bytes_read;
	ADDRESS qemu_port_addr;

	if ((bytes_read = read(fd, data, sizeof(buf))) < 0) {
		E("recvfrom");
		return;
	}

	if (bytes_read < offset) {
		W("bytes_read=%zd", bytes_read);
		return;
	}

	qemu_port_addr.ipproto() = INTEGER(ip6->ip6_nxt);

	incoming_message(OCTETSTRING(bytes_read, data), &qemu_port_addr);
}

void PortQemu::Event_Handler(const fd_set *fds_read,
			     const fd_set */*write_fds*/,
			     const fd_set */*error_fds*/,
			     double /*time_since_last_call*/)
{
	if (FD_ISSET(fd, fds_read))
		Handle_Fd_Event_Readable(fd);
}

#define S_IN_SIZE sizeof(struct sockaddr_in)

const sockaddr *s_in(uint32_t ip, uint16_t port)
{
	static struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ip;
	sin.sin_port = htons(port);

	return (const sockaddr *) &sin;
}

void PortQemu::user_map(const char * /*system_port*/)
{
	fd_set fds_read;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		E("socket");

	if (bind(fd, s_in(src_ip, src_port), S_IN_SIZE) == -1)
		E("bind");

	if (connect(fd, s_in(dst_ip, dst_port), S_IN_SIZE) == -1)
		E("connect");

	FD_ZERO(&fds_read);
	FD_SET(fd, &fds_read);

	Install_Handler(&fds_read, NULL, NULL, 0);
}

void PortQemu::user_unmap(const char * /*system_port*/)
{
	Uninstall_Handler();
	close(fd);
}

int32_t _cs(void *data, size_t data_len)
{
	int32_t s = 0;

	for ( ; data_len > 1; data_len -= 2, data += 2)
		s += *((uint16_t *) data);

	if (data_len)
		s += *((uint8_t *) data);

	return s;
}

uint16_t cs(int32_t s)
{
	return ~((s & 0xFFFF) + (s >> 16));
}

struct vlan_tag {
	uint16_t	vlan_pri: 3,
			vlan_cfi: 1,
			vlan_id: 12;
	uint16_t	type;
};

void PortQemu::outgoing_send(const OCTETSTRING& msg,
			     const ADDRESS *addr)
{
	struct ethhdr *eth = (struct ethhdr *) buf;
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (eth + 1);
	struct tcphdr *tcp = (struct tcphdr *) (ip6 + 1);
	ssize_t data_len = 0;
	int32_t s;

	if (addr && addr->ipproto() == libnetinet::ipproto__t::IPPROTO__RAW) {

		data_len = msg.lengthof();

		if (write(fd, msg, data_len) < data_len)
			E("write");

		return;
	}

	memcpy(&eth->h_source, &src_eth, ETH_ALEN);
	memcpy(&eth->h_dest, &dst_eth, ETH_ALEN);
	eth->h_proto = htons(ETHERTYPE_IPV6);

	if (vlan_enabled) {
		struct vlan_tag *vlan = (struct vlan_tag *) (eth + 1);

		eth->h_proto = htons(ETHERTYPE_VLAN);

		memset(vlan, 0, sizeof(*vlan));
		vlan->type = htons(ETHERTYPE_IPV6);

		data_len += sizeof(*vlan);

		ip6 = (struct ip6_hdr *) (vlan + 1);
		tcp = (struct tcphdr *) (ip6 + 1);
	}

	memset(ip6, 0, sizeof(*ip6));

	ip6->ip6_nxt = IPPROTO_TCP;
	ip6->ip6_plen = htons(msg.lengthof());
	ip6->ip6_vfc = 0x60;
	ip6->ip6_src = src_ip6;
	ip6->ip6_dst = dst_ip6;

	memcpy(tcp, msg, msg.lengthof());

	if (!tcp->th_sum) {

		s = _cs(&ip6->ip6_src, sizeof(struct in6_addr) * 2);

		s += ntohs(ip6->ip6_nxt);

		s += ip6->ip6_plen;

		s += _cs(tcp, msg.lengthof());

		tcp->th_sum = cs(s);
	}

	data_len += msg.lengthof() + sizeof(*eth) + sizeof(*ip6);

	if (write(fd, buf, data_len) < data_len)
		E("write");
}

void PortQemu::outgoing_send(const port__qemu__param& send_par,
			     const ADDRESS *destination_address)
{
	set_parameter(send_par.name(), send_par.val());
}

} /* end of namespace */
