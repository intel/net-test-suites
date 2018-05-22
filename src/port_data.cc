/*
 * Copyright © 2018, Intel Corporation.
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

#include <netinet/ether.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "port_data.hh"

namespace port__data__module {

#define E(fmt, args...)							\
do {									\
	TTCN_error("%s:%s() Error: " fmt "(): %s",			\
		__FILE__, __func__, ## args, strerror(errno));		\
} while (0)

#define _E(fmt, args...)						\
do {									\
	TTCN_error("%s:%s() Error: " fmt ,				\
		__FILE__, __func__, ## args);				\
} while (0)

port__data_PROVIDER::port__data_PROVIDER(const char *name) : PORT(name)
{
	src_ip = dst_ip = inet_addr("127.0.0.1");
	src_port = 7777;
	dst_port = 7771;
}

#define IS_PARAM(_param, _name) (strcmp(_param, _name) == 0)

void port__data_PROVIDER::set_parameter(const char *name, const char *value)
{
	if (IS_PARAM("src_ip", name)) {
		dst_port = inet_addr(value);
		goto out;
	}

	if (IS_PARAM("src_port", name)) {
		src_port = atoi(value);
		goto out;
	}

	if (IS_PARAM("dst_ip", name)) {
		dst_ip = inet_addr(value);
		goto out;
	}

	if (IS_PARAM("dst_port", name)) {
		dst_port = atoi(value);
		goto out;
	}

	E("Unsupported parameter: %s", name);
 out:
	return;
}

void port__data_PROVIDER::Handle_Fd_Event_Readable(int fd)
{
	ssize_t bytes_read = read(fd, buf, sizeof(buf));

	if (bytes_read < 0) {
		E("recvfrom");
		return;
	}

	incoming_message(OCTETSTRING(bytes_read, buf));
}

void port__data_PROVIDER::Event_Handler(const fd_set *fds_read,
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

void port__data_PROVIDER::user_map(const char * /*system_port*/)
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

void port__data_PROVIDER::user_unmap(const char * /*system_port*/)
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

uint16_t inet6_chksum(uint8_t *data, size_t data_len)
{
	struct ethhdr *eth = (struct ethhdr *) data;
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (eth + 1);
	struct icmp6_hdr *icmp6;
	struct tcphdr *tcp;
	struct udphdr *udp;
	uint16_t *sum = NULL;

	if (ntohs(eth->h_proto) == ETHERTYPE_VLAN) {
		ip6 = (struct ip6_hdr *) ((void *) ip6 + 4);
	}

	switch (ip6->ip6_nxt) {
	case IPPROTO_TCP:
		tcp = (struct tcphdr *) (ip6 + 1);
		sum = &tcp->th_sum;
		break;
	case IPPROTO_UDP:
		udp = (struct udphdr *) (ip6 + 1);
		sum = &udp->uh_sum;
		break;
	case IPPROTO_ICMPV6:
		icmp6 = (struct icmp6_hdr *) (ip6 + 1);
		sum = &icmp6->icmp6_cksum;
		break;
	default:
		_E("Unsupported IPPROTO: 0x%04hx (%hu)", ip6->ip6_nxt,
		   ip6->ip6_nxt);
	}

	int32_t s;

	s = _cs(&ip6->ip6_src, sizeof(struct in6_addr) * 2);

	s += htons(ip6->ip6_nxt);

	s += ip6->ip6_plen;

	*sum = 0;

	s += _cs(ip6 + 1, ntohs(ip6->ip6_plen));

	s = cs(s);

	*sum = s;

	return s;
}

void port__data_PROVIDER::outgoing_send(const OCTETSTRING& msg)
{
	ssize_t data_len = msg.lengthof();

	memcpy(buf, msg, data_len);

	inet6_chksum(buf, data_len);

	if (write(fd, buf, data_len) < data_len)
		E("write");
}

} /* end of namespace */
