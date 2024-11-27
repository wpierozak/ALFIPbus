#include <iostream>
#include <thread>
#include <string>
#include <dim/dic.hxx>

using namespace std;

void execRpc(DimRpcInfo* rpcInfo)
{
    int value = 0;

    for (int i = 0; i < 1; i++)
    {
        string *num = new string(to_string(value));
        cout << "[" << rpcInfo->getName() << "]: " << "sending " << *num << "\n";
        rpcInfo->setData((char*)num->c_str());
        value = atoi(rpcInfo->getString());
        cout << "[" << rpcInfo->getName() << "]: " << "received " << value << "\n";
        delete num;
    }
}

int main()
{
    char noLink[7] = "NO RPC";

    DimRpcInfo rpcInfo0("TEST/TEST_0", noLink);
    DimRpcInfo rpcInfo1("TEST/TEST_1", noLink);

    thread thread0(execRpc, &rpcInfo0);
    thread thread1(execRpc, &rpcInfo1);

    thread0.join();
    thread1.join();

    return 0;
}
