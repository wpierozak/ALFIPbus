#ifndef DIMCOMMANDPARALLEL_H
#define DIMCOMMANDPARALLEL_H

#include <dim/dis.hxx>

#include "dimrpcqueue.h"
#include "dimrpcparallel.h"

class DimCommandParallel: public DimCommand
{
public:
    DimCommandParallel(const char *name, const char *format, DimRpcQueue* queue, DimRpcParallel* rpc);

private:
    void commandHandler();
    DimRpcQueue* queue;
    DimRpcParallel* rpc;
};

#endif // DIMCOMMANDPARALLEL_H
