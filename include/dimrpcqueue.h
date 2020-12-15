#ifndef DIMRPCQUEUE_H
#define DIMRPCQUEUE_H

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include <atomic>

#include "dimrpcparallel.h"

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

private:
    std::thread* threadRef;
    std::atomic_bool running;

    std::mutex accessMutex;
    std::condition_variable requestWait;

    void processRequests();
    std::list<Request> requests;
};

#endif // DIMRPCQUEUE_H
