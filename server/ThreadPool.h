#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        // Blokujemy kolejkę tylko na moment pobrania zadania
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        
                        // Czekaj na zadanie lub sygnał stop
                        this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                        
                        if(this->stop && this->tasks.empty()) return;
                        
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task(); // Wykonanie zadania
                }
            });
        }
    }

    // Dodawanie zadań do kolejki
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one(); // Obudź jednego workera
    }

    ~ThreadPool() {
        stop = true;
        condition.notify_all();
        for(std::thread &worker : workers)
            worker.join();
    }
};

#endif