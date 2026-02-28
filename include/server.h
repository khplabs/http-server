#pragma once
#include <thread>
#include <mutex>
#include <vector>

class Server {

    public:
        Server(int port);
        void run();

    private:
        int port;
        int server_fd;

        void accept_connections();
        void client_handler(int client_fd);

        // Tracking threads in this collection
        std::vector<std::thread> threads;
        std::mutex threads_mutex;
};