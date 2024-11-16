#pragma once

#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

template <typename T>
class WSCQueue {
   private:
    std::vector<T> m_vector;
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;

   public:
    void push(T msg, bool save = false) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (save) {
                m_vector.push_back(msg);
            }
            m_queue.push(std::move(msg));
        }
        m_condition.notify_one();
    }

    bool try_pop(T &msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        msg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void wait_and_pop(T &msg) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return !m_queue.empty(); });
        msg = std::move(m_queue.front());
        m_queue.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    std::vector<T> getVector() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_vector;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue = std::queue<T>();
        m_vector.clear();
    }
};
