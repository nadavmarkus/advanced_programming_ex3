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
    T pop();
    void push(T &element);
    void push(std::vector<T> &elements);
};

#endif