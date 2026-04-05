#include "thread_pool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t thread_count, size_t max_queue_size, std::function<void(int)> handler)
    : max_queue_size(max_queue_size), handler(handler), stopping(false) {

        for (size_t i = 0; i < thread_count; i++) {
            workers.emplace_back(&ThreadPool::worker_loop, this);
        }

        std::cout << "Thread pool count: " << thread_count <<"\nWorker queue limit: " << max_queue_size << "\n";
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
    }

    condition.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

bool ThreadPool::enqueue(int client_fd) {
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (queue.size() >= max_queue_size) {
            return false;
        }

        queue.push(client_fd);
    }

    condition.notify_one();

    return true;
}

void ThreadPool::worker_loop() {
    while (true) {
        int client_fd;

        {
            std::unique_lock<std::mutex> lock(mutex);

            condition.wait(lock, [this] {
                return !queue.empty() || stopping;
            });

            if (stopping && queue.empty()) return;

            client_fd = queue.front();
            queue.pop();
        }

        handler(client_fd);
    }
}