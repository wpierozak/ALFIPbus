#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusInterface.h"

#define SIZE 2

int main() {
try {
        srand(time(NULL));

        boost::asio::io_context io_service;
        IPbusTarget target(io_service,"172.20.75.175", 0, 50001);
        target.debug_mode(IPbusTarget::DebugMode::Vital);
        target.timer_tick(std::chrono::seconds(2));
        target.start_timer();
        IPbusControlPacket packet;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::ipread, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\n\n\tRead...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        

        data[0] = std::rand()%0xFFFF; data[1] = std::rand()%0xFFFF;
        std::cout<< "\n\n\tWrite...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;


        packet.addTransaction(TransactionType::ipwrite, address, data, SIZE);
        target.transcieve(packet);

        std::cout << "\n\n\tRead after write...\n\n";

        packet.addTransaction(TransactionType::ipread, address, data, SIZE);
        target.transcieve(packet);
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(5));

        for(int i = 0; i < 2; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            packet.addTransaction(TransactionType::ipread, address, data, SIZE);
            target.transcieve(packet);

            std::cout << "\n\n\tRead...\n\n";
            std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        
        } 
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
