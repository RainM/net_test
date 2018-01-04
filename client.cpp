#include "client.hpp"

#include "common.hpp"

#include <sstream>

#include <unistd.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

client::client(const std::string& host, const std::string& port) {

    struct addrinfo hints = {}, *servinfo = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( ::getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo) != 0) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't get addrinfo for host: " << host << ":" << port);
    }

    for ( addrinfo* ai = servinfo; ai ; ai = ai->ai_next ) {
	socket_ = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (socket >= 0) {
	    if ( ::connect(socket_, ai->ai_addr, ai->ai_addrlen) >= 0) {
		return;
	    }
	}
    }
    __THROW_EXCEPTION_WITH_LOCATION("Can't connect to " << host << ":" << port);
}

std::pair<int64_t, int64_t> client::ping_pong() {
    struct timespec start, write_done, read_done;

    if ( ::clock_gettime(CLOCK_MONOTONIC, &start) == -1 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't get time for START");
    }

    auto written = ::write( socket_,
			    &start,
			    sizeof(start));

    if ( written != sizeof(start) ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't write all data to socket");
    }

    if ( ::clock_gettime(CLOCK_MONOTONIC, &write_done) == -1 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't get time for WRITE_DONE");
    }

    {
	union {
	    struct timespec ts;
	    uint8_t b[sizeof(timespec) / sizeof(uint8_t)];
	} u = {};

	int read_total = 0;
	while (read_total != sizeof(u)) {
	    int bytes_to_read = (int)sizeof(u) - read_total;
	    auto read = ::read(socket_, &u.b[read_total], bytes_to_read);
	    if (read == 0) {
		__THROW_EXCEPTION_WITH_LOCATION("No more data");
	    }
	    read_total += read;
	}
    }

    if ( ::clock_gettime(CLOCK_MONOTONIC, &read_done) == -1 ) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't get time for READ_DONE");
    }

    std::pair<int64_t, int64_t> result = {};

    result.first = write_done.tv_nsec - start.tv_nsec;
    result.first += BILLION * (write_done.tv_sec - start.tv_sec);

    result.second = read_done.tv_nsec - write_done.tv_nsec;
    result.second += BILLION * (read_done.tv_sec - write_done.tv_sec);

    return result;
}

client::~client() {
    ::close(socket_);
}
