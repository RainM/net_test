#pragma once

#include <utility>
#include <string>
#include <cstdint>

struct client {
private:
    int socket_;
    
public:
    client(const std::string& host, int port);
    ~client();

    std::pair<int64_t, int64_t> ping_pong();
};
