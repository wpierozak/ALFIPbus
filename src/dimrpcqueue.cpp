#include "DimRpcParallel/dimrpcqueue.h"

DimRpcQueue::DimRpcQueue()
{
    this->running = true;
    this->threadRef = new std::thread(&DimRpcQueue::processRequests, this);
}

DimRpcQueue::~DimRpcQueue()
{
    this->running = false;
    this->requestWait.notify_one();
    this->threadRef->join();
    delete this->threadRef;
}

void DimRpcQueue::processRequests()
{
    std::mutex requestMutex;
    std::unique_lock<std::mutex> requestLock(requestMutex);

    while (this->running)
    {
        this->requestWait.wait(requestLock);

        while (!this->requests.empty())
        {
            Request request;

            {
                std::lock_guard<std::mutex>(this->accessMutex);
                request = this->requests.front();
                this->requests.pop_front();
            }

            request.referer->processRequest(request.clientId, request.data, request.size);
        }
    }
}

void DimRpcQueue::newRequest(DimRpcParallel* referer, int clientId, void* data, int size)
{
    Request request;
    request.referer = referer;
    request.clientId = clientId;
    request.data = NULL;
    request.size = size;

    if (size > 0)
    {
        request.data = new uint8_t[size];
        memcpy(request.data, data, size);
    }

    std::lock_guard<std::mutex> lock(this->accessMutex);
    this->requests.push_back(request);
    this->requestWait.notify_one();
}
