#pragma once

struct mirror_server {
private:
    int listen_socket_;

public:
    explicit mirror_server(int port);
    ~mirror_server();

    void do_serve();
};
