#include"SwtLink.h"
#include<dim/dis.hxx>
#include<chrono>
#include<thread>
#include<ctime>
int main(int argc, const char** argv)
{
    
    boost::asio::io_context io_context;

    fit_swt::SwtLink target(argv[2], io_context, "127.0.0.1", 50000);

    DimServer::start(argv[1]);
    for(int i = 0; i < std::stoi(argv[3]); i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}