#pragma once

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

namespace stdext::network {
    struct socket_config {
#ifdef _WIN32
        WSADATA wsa_data;
#endif
        sockaddr_in address;
        socklen_t addrlen;
    };

    socket_config* create_config(const std::string& host, int port, uint16_t family = AF_INET);
    void destroy_config(socket_config* config);

    int open_socket(socket_config* config, int type, int protocol);
    int open_socket(uint16_t family, int type, int protocol);
    void close_socket(int fd);

    bool connect(socket_config* config, int client_fd);
    bool bind(socket_config* config, int server_fd);
    bool listen(int server_fd, int backlog);

    int accept(socket_config* config, int server_fd);

    long send(int conn_fd, const char* data, size_t len, int flags = 0);
    long receive(int conn_fd, char* buffer, size_t max_len, int flags = MSG_WAITALL);

    bool is_connected(int fd);
}
