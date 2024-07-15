#include <iostream>
#include "IPbusInterface_NO_QT.h"

int main() {
    boost::asio::io_service io;
    IPbusTarget target(io);
    target.reconnect();
    for(int i = 0; i < 1000; i++)
    {
        std::cout<<"Wait...\n";
        sleep(1);
    }
    return 0;
}
