#include "server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <http_response.h>
#include <http_parser.h>

Server::Server(int port) : port(port), server_fd(-1) {}

void Server::run() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind\n";
        return;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen\n";
        return;
    }

    std::cout << "Listening on port " << port << "\n";
    accept_connections();
}

void Server::accept_connections() {
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }

        char buffer[1024]{};
        read(client_fd, buffer, sizeof(buffer) - 1);

        // Parse the raw request
        HttpParser parser;
        HttpRequest request = parser.parse(std::string(buffer));

        std::cout << "Method: " << request.method
                  << " Path: " << request.path << "\n";

        // Build and send a response
        HttpResponse response;
        std::string body;

        if (request.path == "/") {
            body = "<html><body><h1>Hello from your HTTP server</h1></body></html>";
            std::string raw = response.build(200, body);
            write(client_fd, raw.c_str(), raw.size());
        } else {
            body = "<html><body><h1>404 Not Found</h1></body></html>";
            std::string raw = response.build(404, body);
            write(client_fd, raw.c_str(), raw.size());
        }

        close(client_fd);
    }
}