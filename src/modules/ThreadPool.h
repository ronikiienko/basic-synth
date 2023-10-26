#pragma once

#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include "Vector.h"

struct TaskQueue {
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::atomic<short> tasksLeft = 0;


    template<typename TCallback>
    void addTask(TCallback &&callback) {
        std::lock_guard guard(mutex);
        tasks.push(std::forward<TCallback>(callback));
        tasksLeft++;
    }

    void getTask(std::function<void()> &callback) {
        std::lock_guard guard(mutex);
        if (tasks.empty()) {
            return;
        }
        callback = std::move(tasks.front());
        tasks.pop();
    }

    static void wait() {
        std::this_thread::yield();
    }

    void waitForCompletion() {
        while (tasksLeft > 0) {
            wait();
        }
    }

    void taskDone() {
        tasksLeft--;
    }
};

struct Worker {
    std::thread thread;
    TaskQueue *queue = nullptr;
    bool isRunning = true;
    std::function<void()> task = nullptr;

    explicit Worker(TaskQueue &queue) : queue(&queue) {
        thread = std::thread([this]() {
            run();
        });
    }

    void run() {
        while (isRunning) {
            queue->getTask(task);
            if (task == nullptr) {
                TaskQueue::wait();
            } else {
                task();
                queue->taskDone();
                task = nullptr;
            }
        }
    }

    void stop() {
        isRunning = false;
        thread.join();
    }
};

struct ThreadPool {
    TaskQueue queue;
    std::vector<Worker> workers;
    short threadsNum;

    explicit ThreadPool(short threadsNum) : threadsNum(threadsNum) {
        workers.reserve(threadsNum);
        for (int i = 0; i < threadsNum; i++) {
            workers.emplace_back(queue);
        }
    }

    ~ThreadPool() {
        for (Worker &worker: workers) {
            worker.stop();
        }
    }

    template<typename TCallback>
    void addTask(TCallback &&callback) {
        queue.addTask(std::forward<TCallback>(callback));
    }

    void waitForCompletion() {
        queue.waitForCompletion();
    };

    template<typename TCallback>
    void dispatch(int maxCount, TCallback &&callback) {
        const int batchSize = maxCount / threadsNum;

        for (int i = 0; i < threadsNum; i++) {
            const int start = batchSize * i;
            const int end = start + batchSize;

            addTask([&callback, start, end]() {
                callback(start, end);
            });
        }

        if (batchSize * threadsNum < maxCount) {
            const int start = batchSize * threadsNum;
            const int end = maxCount;
            callback(start, end);
//            addTask([&callback, start, end]() {
//                callback(start, end);
//            });
        }

        waitForCompletion();
    }
};