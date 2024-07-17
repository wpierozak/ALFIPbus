#include <iostream>
#include "IPbusInterface.h"
#include<chrono>
#include<thread>
int main() {
try {
        boost::asio::io_context io_service;
        IPbusTarget target(io_service,"127.0.0.1", 50002, 50001);
         for(int i = 0; i < 3; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        target.stop_timer();
        std::cout<< "\nOK\n";        //io_service.run();  // Run the io_service to process async operations
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
