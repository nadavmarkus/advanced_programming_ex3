
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

#include "BlockingQueue.h"

template <class T>
void BlockingQueue<T>::push(T &element)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    queue.push(element);
    queue_condition.notify_one();
}

template <class T>
void BlockingQueue<T>::push(std::vector<T> &elements)
{
    /*
     * TODO: Consider acquiring the lock once and pushing at once.
     * I should read about the semantics of notify_one of a condition variable.
     */
    for (const auto &element: elements) {
        push(element);
    }
}

template <class T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_condition.wait(lock, []{return !queue.empty(); });
    T to_return = queue.front();
    queue.pop();
    return to_return;
}
