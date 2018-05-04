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

#ifndef PORT_CONS_HH
#define PORT_CONS_HH

#include <queue>

#include <TTCN3.hh>

#include "port_cons_types.hh"

#include <stdint.h>

namespace port__cons__types {

#define PORT_CONS_BUF_MAXSIZE 1024

class port__cons : public port__cons_BASE {
public:
	port__cons(const char *name = NULL) : port__cons_BASE(name) { };
	~port__cons() { };
	void set_parameter(const char *name, const char *value);
protected:
	void user_map(const char *system_port);
	void user_unmap(const char *system_port);
	void outgoing_send(const CHARSTRING& msg);
	void outgoing_send(const port__cons__op__t& op);
	boolean fd_is_line_present(void);
	void Handle_Fd_Event_Readable(int fd);
private:
	int fd;
	FILE *stream;
	uint32_t addr;
	uint16_t port;
	uint8_t buf[PORT_CONS_BUF_MAXSIZE];

	typedef enum {
		CONNECT,
		LISTEN,
		ACCEPT
	} port_cons_op_t;

	std::queue<port_cons_op_t> ops;

	void ops_process(void);
	void op_connect(void);
	void op_listen(void);
	void op_accept(void);
	void stream_getline();
};

} /* end of namespace */

#endif /* PORT_CONS_HH */
