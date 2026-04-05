#include "server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <http_response.h>
#include <http_parser.h>
#include "file_handler.h"
#include <thread>

Server::Server(int port, size_t thread_count, size_t max_queue_size) 
    : port(port), server_fd(-1), pool(thread_count, max_queue_size, [this](int client_fd) {
        client_handler(client_fd);
    }) {}

void Server::run() {

    // Creates the server socket.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    // Sets sockets to reusable.
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET; // Enable IPV4.
    address.sin_addr.s_addr = INADDR_ANY; // Listens on all network interfaces.
    address.sin_port = htons(port); // Maps endianness correctly to network spec.

    // Assign address and port to the socket - fixes address already in use issues.
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind\n";
        return;
    }

    // Makes the socket actively listening with a connection buffer of 10.
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen\n";
        return;
    }

    std::cout << "Listening on port " << port << "\n";
    accept_connections();
}

void Server::accept_connections() {

    while (true) {

        // Assigns a socket to an incoming request.
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd < 0) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }

        if (!pool.enqueue(client_fd)) {
            std::cerr << "Queue has reached maximum capacity, connection refused ;-)";
            close(client_fd);
        }
    }
}

void Server::client_handler(int client_fd) {
    FileHandler file_handler("./static");
    bool keep_alive = true;

    while (keep_alive) {
        char buffer[4096]{};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) break;

        HttpParser parser;
        HttpRequest request = parser.parse(std::string(buffer));

        std::cout << "[thread " << std::this_thread::get_id() << "] "
                  << request.method << " " << request.path << "\n";

        auto it = request.headers.find("connection");
        if (it != request.headers.end() && it->second == "close") {
            keep_alive = false;
        }
            
        // Build and send a response.
        HttpResponse response;
        std::string raw;

        FileResult file = file_handler.read(request.path);

        if (file.found) {
            raw = response.build(200, file.content, keep_alive, file.mime_type);
        } else {
            std::string body = "<html><body><h1>404 Not Found</h1></body><html>";
            raw = response.build(404, body, keep_alive);
        }

         write(client_fd, raw.c_str(), raw.size());
    }

    close(client_fd);
}
