#include"SWTelectronics.h"
#include<dim/dic.hxx>
#include<chrono>
#include<thread>

int main(int argc, const char** argv)
{
    char bad[] = "BAD";
    DimInfo subscribe("TEST/READ/RpcOut", bad);
    std::string cafe = "1004CAFE";
    std::cout<< "Sending comand..." << DimClient::sendCommand("TEST/READ/RpcIn", argv[1]) << std::endl;
    for(int i = 0; i < 5; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string respone = subscribe.getString();
        std::cout<< "Service status: " << respone << std::endl;
    }
    return 0;
}