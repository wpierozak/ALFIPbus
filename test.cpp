#include <iostream>
#include "IPbusInterface_NO_QT.h"

int main() {
    boost::asio::io_service io;
    IPbusTarget target(io);
    target.reconnect();
    return 0;
}
