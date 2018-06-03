
#include <mutex>
#include <condition_variable>

#include "BlockingQueue.h"

template <class T>
void BlockingQueue<T>::push(T &element)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    queue.push(element);
    queue_condition.notify_one();
}

template <class T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_condition.wait(lock, []{return !queue.empty(); });
    T to_return = queue.front();
    queue.pop();
    lock.unlock();
    return to_return;
}
