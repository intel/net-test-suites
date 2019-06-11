/*
 * Copyright © 2018-2019, Intel Corporation.
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "libutils.hh"

#include <sys/types.h>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace libutils {

#define E(fmt, args...)	do {						\
	TTCN_error("%s:%s() Error: " fmt , __FILE__, __func__, ## args);\
} while (0)

INTEGER u32__pre__inc(INTEGER &a, const INTEGER &b)
{
	int64_t a_i64 = a.get_long_long_val(), b_i64 = b.get_long_long_val();

	if (a_i64 & 0xFFFFFFFF00000000) {
		E("Out of range: a: %ld", a_i64);
	}
	if (b_i64 & 0xFFFFFFFF00000000) {
		E("Out of range: b: %ld", b_i64);
	}

	{
		uint32_t a_u32 = a_i64, b_u32 = b_i64;
		a_u32 = a_u32 + b_u32;
		a.set_long_long_val(a_u32);
	}

	return a;
}

INTEGER u32__post__inc(INTEGER &a, const INTEGER &b)
{
	int64_t a_i64 = a.get_long_long_val(), b_i64 = b.get_long_long_val();
	INTEGER c;

	if (a_i64 & 0xFFFFFFFF00000000) {
		E("Out of range: a: %ld", a_i64);
	}
	if (b_i64 & 0xFFFFFFFF00000000) {
		E("Out of range: b: %ld", b_i64);
	}

	c.set_long_long_val(a.get_long_long_val());

	{
		uint32_t a_u32 = a_i64, b_u32 = b_i64;
		a_u32 = a_u32 + b_u32;
		a.set_long_long_val(a_u32);
	}

	return c;
}

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
