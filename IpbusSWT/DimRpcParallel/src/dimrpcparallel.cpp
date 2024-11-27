#include "DimRpcParallel/dimrpcparallel.h"
#include "DimRpcParallel/dimcommandparallel.h"

#include <string>

std::map<int, DimRpcQueue*> DimRpcParallel::rpcQueues;
int DimRpcParallel::rpcCount = 0;

DimRpcParallel::DimRpcParallel(const char *name, const char *formatin, const char *formatout, int bank)
{
    DimRpcParallel::rpcCount++;
    this->inData = NULL;
    this->inSize = 0;
    this->outData = NULL;
    this->outSize = 0;

    this->command = new DimCommandParallel((std::string(name) + "/RpcIn").c_str(), formatin, DimRpcParallel::getRpcQueue(bank), this);

    if (strcmp(formatout, "I") == 0 || strcmp(formatout, "L") == 0)
    {
        int i = 0;
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::INT;
    }
    else if (strcmp(formatout, "F") == 0)
    {
        float i = 0;
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::FLOAT;
    }
    else if (strcmp(formatout, "D") == 0)
    {
        double i = 0;
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::DOUBLE;
    }
    else if (strcmp(formatout, "S") == 0)
    {
        short i = 0;
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::SHORT;
    }
    else if (strcmp(formatout, "X") == 0)
    {
        longlong i = 0;
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::LONGLONG;
    }
    else if (strcmp(formatout, "C") == 0)
    {
        char i[1] = "";
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), i);
        this->serviceType = Type::STRING;
    }
    else
    {
        uint8_t i[1] = { 0 };
        this->service = new DimService((std::string(name) + "/RpcOut").c_str(), const_cast<char*>(formatout), i, 1);
        this->serviceType = Type::POINTER;
    }
}

DimRpcParallel::~DimRpcParallel()
{
    delete this->command;
    delete this->service;

    if (this->inData)
    {
        delete[] (uint8_t*)this->inData;
    }

    if (this->outData)
    {
        delete[] (uint8_t*)this->outData;
    }

    DimRpcParallel::rpcCount--;
    if (!DimRpcParallel::rpcCount)
    {
        DimRpcParallel::deleteRpcQueues();
    }
}

void DimRpcParallel::processRequest(int clientId, void* data, int size)
{
    this->prepareInData(size);

    if (size > 0)
    {
        memcpy(this->inData, data, size);
    }

    if (data)
    {
        delete[] (uint8_t*)data;
    }

    this->rpcHandler();

    int ids[2] = { clientId, 0 };

    switch (this->serviceType)
    {
    case Type::INT:
        this->service->selectiveUpdateService(*(int*)this->outData, ids);
        break;
    case Type::FLOAT:
        this->service->selectiveUpdateService(*(float*)this->outData, ids);
        break;
    case Type::SHORT:
        this->service->selectiveUpdateService(*(short*)this->outData, ids);
        break;
    case Type::DOUBLE:
        this->service->selectiveUpdateService(*(double*)this->outData, ids);
        break;
    case Type::STRING:
        this->service->selectiveUpdateService((char*)this->outData, ids);
        break;
    case Type::POINTER:
        this->service->selectiveUpdateService(this->outData, this->outSize, ids);
        break;
    case Type::LONGLONG:
        this->service->selectiveUpdateService(*(longlong*)this->outData, ids);
        break;
    }
}

void DimRpcParallel::prepareInData(int size)
{
    if (this->inData)
    {
        if (this->inSize != size)
        {
            delete[] (uint8_t*)this->inData;

            if (size <= 0)
            {
                this->inData = NULL;
                this->inSize = 0;
            }

            this->inData = new uint8_t[size];
            this->inSize = size;
        }
    }
    else
    {
        this->inData = new uint8_t[size];
        this->inSize = size;
    }
}

void DimRpcParallel::prepareOutData(int size)
{
    if (this->outData)
    {
        if (this->outSize != size)
        {
            delete[] (uint8_t*)this->outData;

            if (size <= 0)
            {
                this->inData = NULL;
                this->inSize = 0;
            }

            this->outData = new uint8_t[size];
            this->outSize = size;
        }
    }
    else
    {
        this->outData = new uint8_t[size];
        this->outSize = size;
    }
}

DimRpcQueue* DimRpcParallel::getRpcQueue(int bank)
{
    if (!DimRpcParallel::rpcQueues.count(bank))
    {
        DimRpcParallel::rpcQueues[bank] = new DimRpcQueue();
    }

    return DimRpcParallel::rpcQueues[bank];
}

void DimRpcParallel::deleteRpcQueues()
{
    for (std::map<int, DimRpcQueue*>::iterator it = DimRpcParallel::rpcQueues.begin(); it != DimRpcParallel::rpcQueues.end(); it++)
    {
        delete it->second;
    }
}

void *DimRpcParallel::getData()
{
    return this->inData;
}

int DimRpcParallel::getInt()
{
    return *(int*)this->inData;
}

float DimRpcParallel::getFloat()
{
    return *(float*)this->inData;
}

double DimRpcParallel::getDouble()
{
    return *(double*)this->inData;
}

longlong DimRpcParallel::getLonglong()
{
    return *(longlong*)this->inData;
}

short DimRpcParallel::getShort()
{
    return *(short*)this->inData;
}

const char *DimRpcParallel::getString()
{
    return (const char*)this->inData;
}

int DimRpcParallel::getSize()
{
    return this->inSize;
}

void DimRpcParallel::setData(void *data, int size)
{
    this->prepareOutData(size);
    if (size)
    {
        memcpy(this->outData, data, size);
    }
}

void DimRpcParallel::setData(int &data)
{
    this->prepareOutData(sizeof(int));
    memcpy(this->outData, &data, sizeof(int));
}

void DimRpcParallel::setData(float &data)
{
    this->prepareOutData(sizeof(float));
    memcpy(this->outData, &data, sizeof(float));
}

void DimRpcParallel::setData(double &data)
{
    this->prepareOutData(sizeof(double));
    memcpy(this->outData, &data, sizeof(double));
}

void DimRpcParallel::setData(longlong &data)
{
    this->prepareOutData(sizeof(longlong));
    memcpy(this->outData, &data, sizeof(longlong));
}

void DimRpcParallel::setData(short &data)
{
    this->prepareOutData(sizeof(short));
    memcpy(this->outData, &data, sizeof(short));
}

void DimRpcParallel::setData(const char *data)
{
    int size = strlen(data) + 1;
    this->prepareOutData(size);
    memcpy(this->outData, data, size);
}
