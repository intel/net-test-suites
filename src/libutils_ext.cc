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

#include "libutils.hh"

#include <sys/types.h>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace libutils {

CHARSTRING fx__ether__ntoa(OCTETSTRING const &addr)
{
	const uint8_t *d = addr;
	char *s = ether_ntoa((const struct ether_addr *) d);
	return CHARSTRING(strlen(s), s);
}

CHARSTRING fx__inet6__ntop(OCTETSTRING const &addr)
{
	const uint8_t *d = addr;
	char s[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, d, s, sizeof(s));
	return CHARSTRING(strlen(s), s);
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

OCTETSTRING fx__inet__aton(CHARSTRING const &s)
{
	struct in_addr addr;
	inet_aton(s, &addr);
	return OCTETSTRING(sizeof(struct in_addr), (uint8_t *) &addr);
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

INTEGER fx__tcp__csum(OCTETSTRING& msg)
{
	const uint8_t *data = msg;
	struct ip *ip = (struct ip *) data;
	struct tcphdr *tcp = (struct tcphdr *) (ip + 1);
	uint16_t len = ntohs(ip->ip_len) - sizeof(struct ip);
	int32_t s;

	s = _cs(&ip->ip_src, sizeof(struct in_addr) * 2);

	s += htons(ip->ip_p);
	s += htons(len);

	tcp->th_sum = 0;
	s += _cs(tcp, len);

	return cs(s);
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

}
