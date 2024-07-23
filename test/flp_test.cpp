#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusInterface.h"

#define SIZE 2

using namespace IPbus;

int main() {
try {
        srand(time(NULL));

        boost::asio::io_context io_service;
        IPbusTarget target(io_service,"172.20.75.175", 0, 50001);
        target.debugMode(IPbusTarget::DebugMode::Full);
        target.timerTick(std::chrono::seconds(1));
        
        IPbusControlPacket packet;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::data_read, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\n\n\tRead...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        

        data[0] = std::rand()%0xFFFF; data[1] = std::rand()%0xFFFF;
        std::cout<< "\n\n\tWrite...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;


        packet.addTransaction(TransactionType::data_write, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\n\n\tRead after write...\n\n";

        packet.addTransaction(TransactionType::data_read, address, data, SIZE);
        target.transcieve(packet);
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;
        target.startTimer();
        std::this_thread::sleep_for(std::chrono::seconds(5));

        for(int i = 0; i < 2; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            packet.addTransaction(TransactionType::data_read, address, data, SIZE);
            target.transcieve(packet);

            std::cout << "\n\n\tRead...\n\n";
            std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        
        } 
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
