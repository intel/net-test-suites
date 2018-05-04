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

#include "port_cons.hh"
#include "port_cons_types.hh"

#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <unistd.h>

namespace port__cons__types {

#define _E(fmt, args...)						\
do {									\
	TTCN_error("%s:%s() Error: " fmt "(): %s",			\
		__FILE__, __func__, ## args, strerror(errno));		\
} while (0)

#define E(fmt, args...)							\
do {									\
	TTCN_error("%s:%s() Error: " fmt, __FILE__, __func__, ## args);	\
} while (0)

#define D(fmt, args...)							\
do {									\
	TTCN_warning("%s:%s() " fmt, __FILE__, __func__, ## args);	\
} while (0)

#define IS_PARAM(_param, _name) (strcmp(_param, _name) == 0)

void port__cons::set_parameter(const char *name, const char *value)
{
	D("%s=%s", name, value);
	if (IS_PARAM("addr", name)) {
		addr = inet_addr(value);
		goto out;
	}
	if (IS_PARAM("port", name)) {
		port = atoi(value);
		goto out;
	}
	E("Unsupported parameter: %s", name);
out:
	return;
}

#define S_IN_SIZE sizeof(struct sockaddr_in)

static void *s_in(uint32_t addr, uint16_t port)
{
	static struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = htons(port);

	return &sin;
}

void port__cons::op_connect(void)
{
	if (connect(fd, (const struct sockaddr *) s_in(addr, port),
		    S_IN_SIZE) != 0)
		_E("connect");
}

void port__cons::op_listen(void)
{
	int v = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)) == -1)
		_E("setsockopt");

	if (bind(fd, (const struct sockaddr *) s_in(addr, port),
		 	S_IN_SIZE) == -1)
		_E("bind");

	if (listen(fd, 1) == -1)
		_E("listen");
}

void port__cons::op_accept(void)
{
	int conn;
	struct sockaddr_in sin;
	socklen_t sin_size = sizeof(sin);

	memset(&sin, 0, sizeof(sin));

	if ((conn = accept(fd, (struct sockaddr *) &sin, &sin_size)) == -1)
		_E("accept");

	Handler_Remove_Fd_Read(fd);

	fclose(stream);
	close(fd);

	fd = conn;

	if (!(stream = fdopen(fd, "r")))
		_E("fdopen");

	Handler_Add_Fd_Read(fd);
}

void port__cons::ops_process(void)
{
	port_cons_op_t op = ops.front();

	ops.pop();

	switch (op) {
	case CONNECT:
		op_connect();
		break;
	case LISTEN:
		op_listen();
		ops.push(ACCEPT);
		break;
	case ACCEPT:
		op_accept();
		break;
	}
}

boolean port__cons::fd_is_line_present(void)
{
	ssize_t r = recv(fd, &buf, PORT_CONS_BUF_MAXSIZE,
			 MSG_PEEK | MSG_DONTWAIT);
	if (r <= 0) {
		_E("recv");
	}

	return (r > 0) && memchr((const char *) buf, '\n', r) ? TRUE : FALSE;
}

void port__cons::stream_getline()
{
	char *buf = NULL;
	size_t buflen = 0;
	ssize_t len = getline(&buf, &buflen, stream);

	if (len > 0) {
		incoming_message(CHARSTRING(len, buf));
		free(buf);
	}
}

void port__cons::Handle_Fd_Event_Readable(int fd)
{
	if (!ops.empty()) {
		ops_process();
		return;
	}

	if (fd_is_line_present()) {
		stream_getline();
	}
}

void port__cons::user_map(const char * /*system_port*/)
{
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		_E("socket");

	if (!(stream = fdopen(fd, "r")))
		_E("fdopen");

	Handler_Add_Fd_Read(fd);
}

void port__cons::user_unmap(const char * /*system_port*/)
{
	Handler_Remove_Fd_Read(fd);
	fclose(stream);
	close(fd);
}

void port__cons::outgoing_send(const CHARSTRING& msg)
{
	const void *data = msg;
	size_t data_len = msg.lengthof();

	ssize_t bytes_written = write(fd, data, data_len);

	if (bytes_written < data_len)
		_E("write");
}

/* Convenience macro to access port__cons__op__t::enum_type values */
#define _OP(_op) port__cons__op__t::PORT__CONS__OP__ ## _op

void port__cons::outgoing_send(const port__cons__op__t& op)
{
	D("%s(%d)", op.enum_to_str(op), op.enum2int(op));

	switch (op) {
	case _OP(CONNECT):
		ops.push(CONNECT);
		break;
	case _OP(LISTEN):
		ops.push(LISTEN);
		break;
	}

	ops_process();
}

} /* end of namespace */
