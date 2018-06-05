#ifndef __BLOCKING_QUEUE_H_
#define __BLOCKING_QUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

template <class T>
class BlockingQueue
{
private:
    std::queue<T> queue;
    std::mutex queue_mutex;
    std::condition_variable queue_condition;
    
public:
    /* 
     * We return by value to avoid complications.
     *  The underlying object can supply a move semantic copy constructor
     * to enhance performance
     */
    T pop()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_condition.wait(lock, [&]{return !queue.empty(); });
        T to_return = queue.front();
        queue.pop();
        return to_return;
    }
    
    void push(const T &element)
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        queue.push(element);
        queue_condition.notify_one();
    }
    
    void push(std::vector<T> &elements)
    {
        for (const auto &element: elements) {
            push(element);
        }
    }
};

#endif