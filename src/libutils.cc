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

#include "libutils_module.hh"

#include <sys/types.h>

#include <netinet/ether.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <map>
#include <string>

namespace libutils__module {

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

OCTETSTRING fx__ip6__nbr__soliciation__init(void)
{
	uint8_t lla[8];
	unsigned lla_len = 0;
	int lla_enabled = 1;

	struct ethhdr eth;
	struct ip6_hdr ip6;
	struct nd_neighbor_solicit ns;
	struct nd_opt_hdr *ns_lla = (nd_opt_hdr *) lla;

	memset(&lla, 0, sizeof(lla));

	/* ip6_src: link-local (fe80::) */

	const char *ip6_src = "fe80::2";

	/* ip6_dst: solicited node multicast (ff02::1:x:x) */

	const char *ip6_dst = "ff02::1:0:2";
	const char *eth_src = "00:00:00:00:00:02";

	/* eth_dst: l2 multicast: 33:33 + 4 lsb of ip6_src */

	const char *eth_dst = "33:33:00:00:00:02";

	if (lla_enabled) {
		memset(&lla, 0, sizeof(lla));
		ns_lla->nd_opt_type = ND_OPT_SOURCE_LINKADDR;
		ns_lla->nd_opt_len = 1;
		memcpy(&lla[2], ether_aton(eth_src), ETH_ALEN);
		lla_len = sizeof(lla);
	}

	memset(&ns, 0, sizeof(ns));
	ns.nd_ns_hdr.icmp6_type = ND_NEIGHBOR_SOLICIT;
	inet_pton(AF_INET6, ip6_src, (void *) &ns.nd_ns_target);

	memset(&ip6, 0, sizeof(ip6));
	ip6.ip6_vfc= 0x60;
	ip6.ip6_nxt = IPPROTO_ICMPV6;
	ip6.ip6_plen = htons(sizeof(ns) + lla_len);
	inet_pton(AF_INET6, ip6_src, (void *) &ip6.ip6_src);
	inet_pton(AF_INET6, ip6_dst, (void *) &ip6.ip6_dst);

	memcpy(&eth.h_source, ether_aton(eth_src), ETH_ALEN);
	memcpy(&eth.h_dest, ether_aton(eth_dst), ETH_ALEN);
	eth.h_proto = htons(ETHERTYPE_IPV6);

	{
		int32_t s;

		s = _cs(&ip6.ip6_src, sizeof(struct in6_addr) * 2);

		s += ntohs(IPPROTO_ICMPV6 + sizeof(ns) + lla_len);

		if (lla_enabled)
			s += _cs(&lla, sizeof(lla));

		s += _cs(&ns, sizeof(ns));

		ns.nd_ns_hdr.icmp6_cksum = cs(s);
	}

	OCTETSTRING data(sizeof(eth), (uint8_t *) &eth);

	data += OCTETSTRING(sizeof(ip6), (uint8_t *) &ip6);

	data += OCTETSTRING(sizeof(ns), (uint8_t *) &ns);

	data += OCTETSTRING(sizeof(lla), (uint8_t *) &lla);

	return data;
}

void fx__ip6__nbr__soliciation__decode(OCTETSTRING const &msg,
				       OCTETSTRING &ip6_src, OCTETSTRING &lla)
{
	const uint8_t *data = msg;
	struct ethhdr *eth = (struct ethhdr *) data;
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (data + sizeof(struct ethhdr));
	//struct nd_neighbor_solicit *ns = (struct nd_neighbor_solicit *)
	//	(data + sizeof(struct ethhdr) + sizeof(struct ip6_hdr));

	if (ntohs(eth->h_proto) == ETHERTYPE_VLAN) {
		ip6 = (struct ip6_hdr *) ((void *) ip6 + 4);
	}

	ip6_src += OCTETSTRING(sizeof(in6_addr), (uint8_t *) &ip6->ip6_src);

	lla += OCTETSTRING(ETH_ALEN, (uint8_t *) &eth->h_source);
}

OCTETSTRING fx__ip6__nbr__advert__init(OCTETSTRING const &ip6_dst,
				       OCTETSTRING const &eth_dst)
{
	uint8_t lla[8];
	unsigned lla_len = 0;
	int lla_enabled = 1;

	struct ethhdr eth;
	struct ip6_hdr ip6;
	struct nd_neighbor_advert na;
	struct nd_opt_hdr *na_lla = (nd_opt_hdr *) lla;

	const char *eth_src = "00:00:00:00:00:02";
	const char *ip6_src = "fe80::2";

	memset(&lla, 0, sizeof(lla));

	if (lla_enabled) {
		memset(&lla, 0, sizeof(lla));
		na_lla->nd_opt_type = ND_OPT_TARGET_LINKADDR;
		na_lla->nd_opt_len = 1;
		memcpy(&lla[2], ether_aton(eth_src), ETH_ALEN);
		lla_len = sizeof(lla);
	}

	memset(&na, 0, sizeof(na));
	na.nd_na_hdr.icmp6_type = ND_NEIGHBOR_ADVERT;
	memcpy(&na.nd_na_target, ip6_dst, ip6_dst.lengthof());

	memset(&ip6, 0, sizeof(ip6));
	ip6.ip6_vfc= 0x60;
	ip6.ip6_nxt = IPPROTO_ICMPV6;
	ip6.ip6_plen = htons(sizeof(na) + lla_len);
	inet_pton(AF_INET6, ip6_src, (void *) &ip6.ip6_src);
	memcpy(&ip6.ip6_dst, ip6_dst, ip6_dst.lengthof());

	memcpy(&eth.h_source, ether_aton(eth_src), ETH_ALEN);
	memcpy(&eth.h_dest, eth_dst, eth_dst.lengthof());
	eth.h_proto = htons(ETHERTYPE_IPV6);

	{
		int32_t s;

		s = _cs(&ip6.ip6_src, sizeof(struct in6_addr) * 2);

		s += ntohs(IPPROTO_ICMPV6 + sizeof(na) + lla_len);

		if (lla_enabled)
			s += _cs(&lla, sizeof(lla));

		s += _cs(&na, sizeof(na));

		na.nd_na_hdr.icmp6_cksum = cs(s);
	}

	OCTETSTRING data(sizeof(eth), (uint8_t *) &eth);

	data += OCTETSTRING(sizeof(ip6), (uint8_t *) &ip6);

	data += OCTETSTRING(sizeof(na), (uint8_t *) &na);

	data += OCTETSTRING(sizeof(lla), (uint8_t *) &lla);

	return data;
}

static unsigned char ping[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0xdd, 0x60, 0x07,
	0xb4, 0x13, 0x00, 0x40, 0x3a, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x00, 0x17, 0x63, 0x00, 0x01
};

OCTETSTRING fx__ip6__echo__init(OCTETSTRING const &eth_dst,
				OCTETSTRING const &eth_src,
				OCTETSTRING const &ip6_dst,
				OCTETSTRING const &ip6_src)
{
	struct ethhdr *eth = (struct ethhdr *) ping;
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (eth + 1);
	struct icmp6_hdr *icmp6 = (struct icmp6_hdr *) (ip6 + 1);

	memcpy(&eth->h_dest, eth_dst, eth_dst.lengthof());
	memcpy(&eth->h_source, eth_src, eth_src.lengthof());

	memcpy(&ip6->ip6_dst, ip6_dst, ip6_dst.lengthof());
	memcpy(&ip6->ip6_src, ip6_src, ip6_src.lengthof());

	ip6->ip6_plen = htons(8);

	{
		int32_t s;

		s = _cs(&ip6->ip6_src, sizeof(struct in6_addr) * 2);

		s += htons(ip6->ip6_nxt);

		s += ip6->ip6_plen;

		icmp6->icmp6_cksum = 0;

		s += _cs(icmp6, ntohs(ip6->ip6_plen));

		icmp6->icmp6_cksum = cs(s);
	}

	OCTETSTRING data(sizeof(ping), ping);

	return data;
}

OCTETSTRING fx__ip6__init(OCTETSTRING const &eth_dst,
			  OCTETSTRING const &eth_src,
			  OCTETSTRING const &ip6_dst,
			  OCTETSTRING const &ip6_src,
			  libnetinet::ipproto__t const& nxt,
			  INTEGER const& plen)
{
	struct ethhdr eth;
	struct ip6_hdr ip6;

	memcpy(&eth.h_dest, eth_dst, eth_dst.lengthof());
	memcpy(&eth.h_source, eth_src, eth_src.lengthof());
	eth.h_proto = htons(ETHERTYPE_IPV6);

	memset(&ip6, 0, sizeof(ip6));
	ip6.ip6_vfc= 0x60;

	memcpy(&ip6.ip6_dst, ip6_dst, ip6_dst.lengthof());
	memcpy(&ip6.ip6_src, ip6_src, ip6_src.lengthof());

	ip6.ip6_nxt = nxt.as_int();
	ip6.ip6_plen = htons(plen);

	OCTETSTRING data(sizeof(eth), (uint8_t *) &eth);

	data += OCTETSTRING(sizeof(ip6), (uint8_t *) &ip6);

	return data;
}

CHARSTRING fx__ether__ntoa(OCTETSTRING const &addr)
{
	const uint8_t *d = addr;
	char *s = ether_ntoa((const struct ether_addr *) d);
	return CHARSTRING(strlen(s) , s);
}

CHARSTRING fx__inet6__ntop(OCTETSTRING const &addr)
{
	const uint8_t *d = addr;
	char s[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, d, s, sizeof(s));
	return CHARSTRING(strlen(s) , s);
}

OCTETSTRING fx__ether__aton(CHARSTRING const &s)
{
	struct ether_addr *addr = ether_aton(s);
	return OCTETSTRING(sizeof(struct ether_addr), (uint8_t *) addr);
}

OCTETSTRING fx__inet6__pton(CHARSTRING const &s)
{
	struct in6_addr addr;
	inet_pton(AF_INET6, s, &addr);
	return OCTETSTRING(sizeof(struct in6_addr), (uint8_t *) &addr);
}

INTEGER fx__inet6__chksum(const OCTETSTRING& msg)
{
	const uint8_t *data = msg;
	struct ethhdr *eth = (struct ethhdr *) data;
	struct ip6_hdr *ip6 = (struct ip6_hdr *) (eth + 1);
	struct tcphdr *tcp = (struct tcphdr *) (ip6 + 1);

	int32_t s;

	s = _cs(&ip6->ip6_src, sizeof(struct in6_addr) * 2);

	s += htons(ip6->ip6_nxt);

	s += ip6->ip6_plen;

	tcp->th_sum = 0;

	s += _cs(tcp, ntohs(ip6->ip6_plen));

	return cs(s);
}

static std::map<std::string, std::string> fx__db;

void fx__db__insert(CHARSTRING const &cs_key, CHARSTRING const &cs_value)
{
	std::string key((const char *) cs_key);
	std::string value((const char *) cs_value);

	fx__db[key] = value;
}

CHARSTRING fx__db__search(CHARSTRING const &cs_key)
{
	std::string key((const char *) cs_key);

	/* TODO: Handle missing key */

	std::string value = fx__db[key];

	return CHARSTRING(value.c_str());
}

}
