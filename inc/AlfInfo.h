#pragma once
#include "dimrpcparallel.h"
#include<string>

class AlfInfo: public DimRpcParallel
{
public:
    AlfInfo(std::string_view rpc):  DimRpcParallel(rpc.data(), "C", "C", 0) {}
    void rpcHandler();
    bool isMetadataFileAvailable();
private:
    static constexpr std::string_view MetadataFile = "alfmetadata.txt";
};