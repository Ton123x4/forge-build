#include "stdext/network.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#endif

#ifndef _WIN32
#define closesocket close
#endif

namespace stdext::network {
    socket_config* create_config(const std::string& host, int port, uint16_t family) {
        auto config = new socket_config();

#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &config->wsa_data) != 0) {
            delete config;
            return nullptr;
        }
#endif

        config->address.sin_family = family;
        config->address.sin_port = htons(port);
        config->addrlen = sizeof(config->address);

        if (inet_pton(family, host.c_str(), &config->address.sin_addr) <= 0) {
            destroy_config(config);
            return nullptr;
        }

        return config;
    }

    void destroy_config(socket_config* config) {
#ifdef _WIN32
        WSACleanup();
#endif

        delete config;
    }

    int open_socket(socket_config* config, int type, int protocol) {
        return socket(config->address.sin_family, type, protocol);
    }

    int open_socket(uint16_t family, int type, int protocol) {
        return socket(family, type, protocol);
    }

    void close_socket(int fd) {
        closesocket(fd);
    }

    bool connect(socket_config* config, int client_fd) {
        return ::connect(client_fd, (sockaddr*)(&config->address), config->addrlen) >= 0;
    }

    bool bind(socket_config* config, int server_fd) {
        return ::bind(server_fd, (sockaddr*)(&config->address), config->addrlen) >= 0;
    }

    bool listen(int server_fd, int backlog) {
        return ::listen(server_fd, backlog) >= 0;
    }

    int accept(socket_config* config, int server_fd) {
        return ::accept(server_fd, (sockaddr*)&config->address, &config->addrlen);
    }

    long send(int conn_fd, const char* data, size_t len, int flags) {
        return ::send(conn_fd, data, len, flags);
    }

    long receive(int conn_fd, char* buffer, size_t max_len, int flags) {
        return ::recv(conn_fd, buffer, max_len, flags);
    }

    // Non-blocking liveness check: poll() with a zero timeout first, so this
    // never stalls waiting for data. POLLHUP/POLLERR means the peer is gone.
    // Otherwise a non-blocking MSG_PEEK confirms the socket is still usable:
    // 0 means the peer closed cleanly, EAGAIN/EWOULDBLOCK means it's alive
    // with nothing to read yet, and anything else is a dead connection.
    bool is_connected(int fd) {
        if (fd < 0) {
            return false;
        }

#ifdef _WIN32
        WSAPOLLFD poll_fd;

        poll_fd.fd = fd;
        poll_fd.events = POLLRDNORM;
        poll_fd.revents = 0;

        auto poll_result = WSAPoll(&poll_fd, 1, 0);
#else
        pollfd poll_fd;

        poll_fd.fd = fd;
        poll_fd.events = POLLIN;
        poll_fd.revents = 0;

        auto poll_result = ::poll(&poll_fd, 1, 0);
#endif

        if (poll_result < 0) {
            return false;
        }

        if (poll_result == 0) {
            return true;
        }

#ifdef _WIN32
        if (poll_fd.revents & (POLLHUP | POLLERR)) {
            return false;
        }
#else
        if (poll_fd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
            return false;
        }
#endif

        char buffer;
        auto peek_result = ::recv(fd, &buffer, sizeof(buffer), MSG_PEEK);

        if (peek_result == 0) {
            return false;
        }

        if (peek_result < 0) {
#ifdef _WIN32
            return false;
#else
            return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
        }

        return true;
    }
}
