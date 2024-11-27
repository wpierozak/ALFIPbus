#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusMaster.h"

#define SIZE 2

using namespace ipbus;
using namespace ipbus::enums::transactions;
int main() {
try {
        srand(time(NULL));

        boost::asio::io_context io_service;
        IPbusMaster target(io_service,"172.20.75.175", 0, 50001);
        //target.timerTick(std::chrono::seconds(1));
        
        IPbusRequest packet;
        IPbusResponse response;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        packet.addTransaction(TransactionType::Read, address, data, data, SIZE);
        target.transceive(packet, response);
        packet.reset();
        response.reset();

        std::cout << "\n\n\tRead...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        

        data[0] = std::rand()%0xFFFF; data[1] = std::rand()%0xFFFF;
        std::cout<< "\n\n\tWrite...\n\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;


        packet.addTransaction(TransactionType::Write, address, data, data, SIZE);
        target.transceive(packet, response);
        
        packet.reset();
        response.reset();

        std::cout << "\n\n\tRead after write...\n\n";
        data[0] = 0; data[1] = 0;
        packet.addTransaction(TransactionType::Read, address, data, data, SIZE);
        target.transceive(packet, response);
        packet.reset();
        response.reset();

        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;
       // target.startTimer();
        std::this_thread::sleep_for(std::chrono::seconds(5));

        for(int i = 0; i < 2; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            packet.addTransaction(TransactionType::Read, address, data, data, SIZE);
            target.transceive(packet,response);
            packet.reset();
            response.reset();

            std::cout << "\n\n\tRead...\n\n";
            std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;        
        } 
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}
