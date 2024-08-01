#include"IPbusSlave.h"

namespace ipbus
{
    IPbusSlave::IPbusSlave(boost::asio::io_context& io, Memory* memory, uint16_t lport):
        m_ioContext(io),
        m_memory(memory),
        m_socket(io),
        m_remoteEndpoint(boost::asio::ip::udp::v4(), lport)
    {
        if(openSocket())
        {
            startAsyncRecv();
        }
    }

    bool IPbusSlave::openSocket()
    {
        BOOST_LOG_TRIVIAL(debug) << "Attempting to open socket...";
        m_socket.open(boost::asio::ip::udp::v4());

        if (m_socket.is_open()) {
            BOOST_LOG_TRIVIAL(info) << "Socket at " << m_ipAddress << ":" << m_remotePort << " successfully opened";
            return true;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Failed to open socket at " << m_ipAddress << ":" << m_localPort;
            return false;
        }
    }

    bool IPbusSlave::startAsyncRecv()
    {
        m_socket.async_recv_from(boost::asio::buffer(m_request.getSize(), maxSize), m_remoteEndpoint,
                                boost::bind(&IPbusSlave::handleRequest, this, std::placeholders::_1, std::placeholders::_2));
    }

    void IPbusSlave::handleRequest(const boost::system::error_code& ec, std::size_t length)
    {
        m_request.setSize(length);
        m_response.reset();

        uint16_t idx_request = 1;
        uint16_t idx_response = 1;

        for(uint16_t idx = 0; idx < m_request.getTransactionNumber(); idx++)
        {
            TransactionHeader* headerRequest = (TransactionHeader*) m_request.getBuffer() + (idx_request++);
            switch(headerRequest->type)
            {
                case DataRead:
                {
                    InfoCode code = memory->dataRead(m_request[idx_request], headerRequest->words, m_buffer);
                    
                }
                break;
            }
        }
    }

    void IPbusSlave::sendResponse()
    {

    }

    InfoCode IPbusSlave::read(uint32_t address, uint8_t words, uint32_t* out)
    {

    }

    InfoCode IPbusSlave::write(uint32_t address, uint8_t words, uint32_t* in)
    {

    }

    InfoCode IPbusSlave::RMWbits(uint32_t address, uint32_t and, uint32_t or)
    {

    }

    InfoCode IPbusSlave::RMWsum(uint32_t address, uint32_t add)
    {

    }

    InfoCode IPbusSlave::readNonIncrement(uint32_t address, uint8_t words, uint32_t* out)
    {

    }

    InfoCode IPbusSlave::writeNonIncrement(uint32_t address, uint8_t words, uint32_t* in)
    {

    }
} // namespace ipbus

