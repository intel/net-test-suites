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

#ifndef PortQemu_HH
#define PortQemu_HH

#include <sys/types.h>

#include <netinet/ether.h>
#include <netinet/ip6.h>

#include <stdint.h>

#include "PortQemu_Module.hh"

namespace PortQemu__Module {

#define PORT_QEMU_BUF_MAXSIZE 1522

class PortQemu : public PortQemu_BASE {
public:
	PortQemu(const char *port_name = NULL);
	~PortQemu();
	void set_parameter(const char *parameter_name,
			   const char *parameter_value);
	void Event_Handler(const fd_set *read_fds,
			   const fd_set *write_fds,
			   const fd_set *error_fds,
			   double time_since_last_call);
protected:
	void user_map(const char *system_port);
	void user_unmap(const char *system_port);
	void outgoing_send(const OCTETSTRING& msg,
			   const ADDRESS * /*destination_address*/);
	void outgoing_send(const port__qemu__param& send_par, const ADDRESS *destination_address);
private:
	int fd;

	uint8_t	src_eth[ETH_ALEN];
	uint32_t src_ip;
	struct in6_addr src_ip6;
	uint16_t src_port;

	uint8_t dst_eth[ETH_ALEN];
	uint32_t dst_ip;
	struct in6_addr dst_ip6;
	uint16_t dst_port;

	boolean vlan_enabled = false;

	uint8_t buf[PORT_QEMU_BUF_MAXSIZE];

	void Handle_Fd_Event_Readable(int fd);
};

} /* end of namespace */

#endif /* PortQemu_HH */
