#ifndef INCLUDED_DBG_BLOCKING_QUEUE_H
#define INCLUDED_DBG_BLOCKING_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <class T> 
class blocking_queue {
public:
  blocking_queue() = default;

  void push(const T& data) {
    {
      std::unique_lock<std::mutex> lock(mu_);
      queue_.push(data);
    }
    cv_.notify_one();
  }

  [[nodiscard]] T pop() {
    std::unique_lock<std::mutex> lock(mu_);
    while (queue_.empty()) {
      cv_.wait(lock);
    }
    T val = queue_.front();
    queue_.pop();
    return val;
  }

  [[nodiscard]] bool empty() { return queue_.empty(); }

  [[nodiscard]] size_t size() { return queue_.size(); }

private:
  std::queue<T> queue_;
  std::condition_variable cv_;
  mutable std::mutex mu_;
};


#endif  // INCLUDED_DBG_H


