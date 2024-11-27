#ifndef DIMRPCQUEUE_H
#define DIMRPCQUEUE_H

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include <atomic>

#include "DimRpcParallel/dimrpcparallel.h"

class DimRpcQueue
{
public:
    DimRpcQueue();
    ~DimRpcQueue();

    void newRequest(DimRpcParallel* referer, int clientId, void* data, int size);

    struct Request
    {
        DimRpcParallel* referer;
        int clientId;
        void* data;
        int size;
    };

    class QueueLock
    {
    private:
        std::mutex lock;
        std::condition_variable condition;
        uint32_t available, capacity;

    public:
        QueueLock(uint32_t capacity = 0xFFFFFFFF);
        ~QueueLock();

        void wait();
        void notify();
    };

private:
    std::thread* threadRef;
    std::atomic_bool running;

    std::mutex accessMutex;
    QueueLock queueLock;

    void processRequests();
    std::list<Request> requests;
};

#endif // DIMRPCQUEUE_H
