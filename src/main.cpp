#include "server.h"
#include <thread>

int main() {
    size_t thread_count = std::thread::hardware_concurrency() * 2;
    size_t max_queue_size = 100;

    Server server(8080, thread_count, max_queue_size);
    server.run();
    return 0;
}