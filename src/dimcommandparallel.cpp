#include "DimRpcParallel/dimcommandparallel.h"

DimCommandParallel::DimCommandParallel(const char *name, const char *format, DimRpcQueue *queue, DimRpcParallel *rpc): DimCommand(name, format)
{
    this->queue = queue;
    this->rpc = rpc;
}

void DimCommandParallel::commandHandler()
{
    this->queue->newRequest(this->rpc, DimServer::getClientId(), getData(), getSize());
}
