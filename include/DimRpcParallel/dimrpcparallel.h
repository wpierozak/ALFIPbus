#ifndef DIMRPCPARALLEL_H
#define DIMRPCPARALLEL_H

#include <dim/dis.hxx>
#include <map>

class DimCommandParallel;
class DimRpcQueue;

class DimRpcParallel
{
    friend class DimRpcQueue;

public:
    DimRpcParallel(const char *name, const char *formatin, const char *formatout, int bank);
    virtual ~DimRpcParallel();

    void *getData();
    int getInt();
    float getFloat();
    double getDouble();
    longlong getLonglong();
    short getShort();
    const char *getString();
    int getSize();

    void setData(void *data, int size);
    void setData(int &data);
    void setData(float &data);
    void setData(double &data);
    void setData(longlong &data);
    void setData(short &data);
    void setData(const char *data);

    virtual void rpcHandler() = 0;

private:
    DimCommandParallel* command;
    DimService* service;

    void *inData, *outData;
    int inSize, outSize;

    void prepareInData(int size);
    void prepareOutData(int size);

    void processRequest(int clientId, void* data, int size);

    static int rpcCount;
    static std::map<int, DimRpcQueue*> rpcQueues;
    static DimRpcQueue* getRpcQueue(int bank);
    static void deleteRpcQueues();

    enum Type
    {
        INT,
        FLOAT,
        DOUBLE,
        SHORT,
        LONGLONG,
        STRING,
        POINTER
    };

    Type serviceType;
};

#endif // DIMRPCPARALLEL_H
