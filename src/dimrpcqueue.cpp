#include "DimRpcParallel/dimrpcqueue.h"

DimRpcQueue::QueueLock::QueueLock(uint32_t capacity)
{
    this->available = 0;
    this->capacity = capacity;
}

DimRpcQueue::QueueLock::~QueueLock()
{
    notify();
}

void DimRpcQueue::QueueLock::wait()
{
    std::unique_lock<std::mutex> guard(this->lock);
    while (!this->available)
    {
        this->condition.wait(guard);
    }
    --this->available;
}

void DimRpcQueue::QueueLock::notify()
{
    std::lock_guard<std::mutex> guard(this->lock);
    if (this->available < this->capacity)
    {
        ++this->available;
    }
    this->condition.notify_one();
}

DimRpcQueue::DimRpcQueue()
{
    this->running = true;
    this->threadRef = new std::thread(&DimRpcQueue::processRequests, this);
}

DimRpcQueue::~DimRpcQueue()
{
    this->running = false;
    queueLock.notify();
    this->threadRef->join();
    delete this->threadRef;
}

void DimRpcQueue::processRequests()
{
    while (this->running)
    {
        queueLock.wait();

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
    queueLock.notify();
}
