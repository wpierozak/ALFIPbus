#include <iostream>
#include "IPbusInterface_NO_QT.h"

int main() {
try {
        boost::asio::io_service io_service;
        IPbusTarget target(io_service);
        
        target.reconnect();

        io_service.run();  // Run the io_service to process async operations
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
