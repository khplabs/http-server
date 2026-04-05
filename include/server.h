#pragma once
#include <thread_pool.h>
#include <string>

class Server {

    public:
        Server(int port, size_t thread_count, size_t max_queue_size);
        void run();

    private:
        int port;
        int server_fd;

        ThreadPool pool;

        void accept_connections();
        void client_handler(int client_fd);
};