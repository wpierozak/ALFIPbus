#include <iostream>
#include<chrono>
#include<thread>
#include<ctime>
#include "IPbusMaster.h"
#include "IPbusRequest.h"
#include "IPbusResponse.h"

#define SIZE 2

int main() {
    try {
        srand(time(NULL));

        boost::asio::io_context io;
        IPbusMaster target(io,"172.20.75.175", 0, 50001);
        IPbusRequest request;
        IPbusResponse response;

        uint32_t data[SIZE] = {0x0,0x0};
        uint32_t address = 0x1004;

        request.addTransaction(TransactionType::DataRead, address, data, SIZE);
        target.transceive(request, response);

        std::cout << "\nRead...\n";
        std::cout << std::hex << data[0] << ' ' << std::hex << data[1] << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    };
    return 0;
}