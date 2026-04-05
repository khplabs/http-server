#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
    public:
        ThreadPool(size_t thread_count, size_t max_queue_size, std::function<void(int)>);
        ~ThreadPool();

        bool enqueue(int client_fd);

    private:
        size_t max_queue_size;
        std::function<void(int)> handler;

        std::vector<std::thread> workers;
        std::queue<int> queue;

        std::mutex mutex;
        std::condition_variable condition;
        bool stopping;

        void worker_loop();
};