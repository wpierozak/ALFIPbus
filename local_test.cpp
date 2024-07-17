#include <iostream>
#include "IPbusInterface.h"

int main() {
try {
        boost::asio::io_context io_service;
        IPbusTarget target(io_service,"127.0.0.1", 50002, 50001);
        io_service.run();
        //io_service.run();  // Run the io_service to process async operations
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
