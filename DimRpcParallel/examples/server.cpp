#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <mutex>
#include <sys/time.h>
#include "DimRpcParallel/dimrpcparallel.h"

using namespace std;

static mutex coutLock;
static double startTime = 0;

static double getTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double val = tv.tv_sec * 1000 + tv.tv_usec / 1000.0;

    if (startTime == 0)
    {
        startTime = val;
    }

    return val - startTime;
}

class DimRpcTest: public DimRpcParallel
//class DimRpcTest: public DimRpc
{
    void rpcHandler()
    {
        {
            lock_guard<mutex> lock(coutLock);
            cout << "[" << name << "]: " << "handler started: " << getTime() << "\n";
        }

        int value = atoi(getString());

        for (int i = 1; i <= 10; i++)
        {
            {
                lock_guard<mutex> lock(coutLock);
                cout << "[" << name << "]: " << "stage " << i << "\n";
            }
            usleep(1000);
        }

        setData((char*)to_string(value + 1).c_str());

        {
            lock_guard<mutex> lock(coutLock);
            cout << "[" << name << "]: " << "handler ended: " << getTime() << "\n";
        }
    }

    string name;

public:
    DimRpcTest(string name, int bank): DimRpcParallel(name.c_str(), "C", "C", bank)
    //DimRpcTest(string name, int bank): DimRpc(name.c_str(), "C", "C")
    {
        this->name = name;
    }
};

static bool running = true;

void ctrlC(int)
{
    running = false;
}

int main()
{
    signal(SIGINT, ctrlC);

    DimRpcTest test0("TEST/TEST_0", 0);
    DimRpcTest test1("TEST/TEST_1", 1);

    DimServer::start("TEST");

    while (running)
    {
        usleep(10);
    }

    return 0;
}
