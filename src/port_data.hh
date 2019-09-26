/*
 * Copyright © 2018-2019, Intel Corporation.
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

#ifndef PORT_DATA_HH
#define PORT_DATA_HH

#include <TTCN3.hh>

#include <stdint.h>

namespace port__data__types {

#define PORT_DATA_BUF_MAXSIZE 1522

class port__data_PROVIDER : public PORT {
public:
	port__data_PROVIDER(const char *port_name = NULL);
	~port__data_PROVIDER() { }
	void set_parameter(const char *parameter_name,
			   const char *parameter_value);
	void Event_Handler(const fd_set *read_fds,
			   const fd_set *write_fds,
			   const fd_set *error_fds,
			   double time_since_last_call);
protected:
	void user_map(const char *system_port);
	void user_unmap(const char *system_port);
	void outgoing_send(const OCTETSTRING& msg);
	virtual void incoming_message(const OCTETSTRING& incoming_par) = 0;
private:
	int fd;
	int debug;
	uint32_t src_ip;
	uint16_t src_port;
	uint32_t dst_ip;
	uint16_t dst_port;
	uint8_t buf[PORT_DATA_BUF_MAXSIZE];

	void Handle_Fd_Event_Readable(int fd);
};

} /* end of namespace */

#endif /* PORT_DATA_HH */
