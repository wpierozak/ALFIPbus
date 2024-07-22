#include"ALFIPbus.hpp"
#include<chrono>
#include<thread>
#include<ctime>
int main(int argc, const char** argv)
{
    if(argc < 2) return -1;

    ALFIPbus alf(argv[1]);
    alf.init_link(argv[2], std::stoi(argv[3]));
    alf.start_server();
    return 0;
}