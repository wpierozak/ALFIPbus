#include"SWTelectronics.h"
#include<dim/dis.hxx>
#include<chrono>
#include<thread>
#include<ctime>
int main(int argc, const char** argv)
{
    
    boost::asio::io_context io_service;

    SWTelectronics target(argv[2], io_service);
    target.debug_mode(IPbusTarget::DebugMode::Full);
    target.timer_tick(std::chrono::seconds(1));
    target.start_timer();

    DimServer::start(argv[1]);
    for(int i = 0; i < std::stoi(argv[3]); i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}