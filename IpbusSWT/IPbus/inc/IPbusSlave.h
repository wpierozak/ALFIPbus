#include"IPbusRequest.h"
#include"IPbusResponse.h"
#include"IPbusStatusPacket.h"
#include"Memory.h"
#include<boost/asio.hpp>
#include <functional>

#ifndef IPBUS_SLAVE
#define IPBUS_SLAVE

namespace ipbus
{
    class IPbusSlave
    {
       public:
        IPbusSlave(boost::asio::io_context& io, Memory& memory, uint16_t lport);
        bool openSocket();
        void startAsyncRecv();

        const IPbusRequest& getRequest() { return m_request; }
        const IPbusResponse& getResponse() { return m_response; }

        static constexpr uint8_t BufferSize = UINT8_MAX;
    
        void setRequestCallback(std::function<void(const IPbusRequest& req)> f);
       private:

        void handleRequest(const boost::system::error_code& ec, std::size_t length);
        void sendResponse(const boost::asio::ip::udp::endpoint&);
        void sendStatusResponse(const boost::asio::ip::udp::endpoint&);

        InfoCode read(uint32_t address, uint8_t words, uint32_t* out);
        InfoCode write(uint32_t address, uint8_t words, uint32_t* in);
        InfoCode rmwBits(uint32_t address, uint32_t andMask, uint32_t orMask);
        InfoCode rmwSum(uint32_t address, uint32_t add);
        InfoCode readNonIncrement(uint32_t address, uint8_t words, uint32_t* out);
        InfoCode writeNonIncrement(uint32_t address, uint8_t words, uint32_t* in);

        boost::asio::io_context& m_ioContext;
        uint16_t m_localPort;
        
        boost::asio::ip::udp::socket m_socket;
        boost::asio::ip::udp::endpoint m_remoteEndpoint;
        boost::asio::ip::udp::endpoint m_localEndpoint;

        StatusPacket m_statusRequest;
        StatusPacket m_statusResponse;

        IPbusResponse m_response;
        IPbusRequest m_request;

        bool isAsyncRecv{false};

        uint32_t m_buffer[BufferSize];

        Memory& m_memory;

        std::function<void(const IPbusRequest& req)> m_requestCallback;
    };
}

#endif 