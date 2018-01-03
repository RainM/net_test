#include "server.hpp"
#include "common.hpp"

#include <sstream>

#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h>

mirror_server::mirror_server(int port) {
    listen_socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if ( listen_socket_ < 0 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't create socket")
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ( ::bind(listen_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't bind socket to port " << port);
    }

    if ( ::listen( listen_socket_, 0 ) != 0 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't start listening");
    }
}

mirror_server::~mirror_server() {
    
}

void mirror_server::do_serve() {
    int connection_fd = ::accept(listen_socket_, NULL, NULL);
    if (connection_fd < 0) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't accept connection");
    }

    for ( ; ; ) {
	union {
	    struct timespec ts;
	    uint8_t b[sizeof(timespec) / sizeof(uint8_t)];
	} u = {};

	int read_total = 0;
	while (read_total != sizeof(u)) {
	    int bytes_to_read = (int)sizeof(u) - read_total;
	    auto read = ::read(connection_fd, &u.b[read_total], bytes_to_read);
	    if (read == 0) {
		__THROW_EXCEPTION_WITH_LOCATION("No more data");
	    }
	    read_total += read;
	}

	auto written = ::write(connection_fd, u.b, sizeof (u));

	if ( written != sizeof(u) ) {
	    ::close(connection_fd);
	    __THROW_EXCEPTION_WITH_LOCATION("Can't write all data to socket");
	}
    }
    
    ::close(connection_fd);
}
