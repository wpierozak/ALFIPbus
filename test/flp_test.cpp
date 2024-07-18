#include <iostream>
#include<chrono>
#include<thread>
#include "IPbusInterface.h"

int main() {
try {
        boost::asio::io_context io_service;
        IPbusTarget target(io_service,"172.20.75.175", 0, 50001);
        target.debug_mode(IPbusTarget::DebugMode::Full);
        IPbusControlPacket packet;

        uint32_t data[2] = {std::rand()%0xFFFF, std::rand()%0xFFFF};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::ipread, address, data, 2);
        target.transcieve(packet);

        std::cout << "\nRead...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        

        data[0] = std::rand()%0xFFFF; data[1] = std::rand()%0xFFFF;
        std::cout<< "\nWrite...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;

        packet.addTransaction(TransactionType::ipwrite, address, data, 2);
        target.transcieve(packet);

        std::cout << "\nRead after write...\n";

        packet.addTransaction(TransactionType::ipread, address, data, 2);
        target.transcieve(packet);
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;
        
        for(int i = 0; i < 10; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        target.stop_timer();
        std::cout<< "\nOK\n";    
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
