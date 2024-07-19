#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusInterface.h"

#define SIZE 2

int main() {
    try {
        srand(time(NULL));

        boost::asio::io_context io;
        IPbusTarget target(io,"172.20.75.175", 0, 50001);
        IPbusControlPacket packet;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::data_read, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\nRead...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}