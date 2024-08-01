#include"IPbusSlave.h"

namespace ipbus
{
    IPbusSlave::IPbusSlave(boost::asio::io_context& io, Memory* memory, uint16_t lport):
        m_ioContext(io),
        m_memory(memory),
        m_socket(io),
        m_localPort(lport),
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
        m_socket.open(boost::asio::ip::udp::v4(), m_localPort);

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
        m_socket.async_recv_from(boost::asio::buffer(m_request.getSize(), maxPacket*wordSize), m_remoteEndpoint,
                                boost::bind(&IPbusSlave::handleRequest, this, std::placeholders::_1, std::placeholders::_2));
    }

    void IPbusSlave::handleRequest(const boost::system::error_code& ec, std::size_t length)
    {
        boost::asio::ip::udp::endpoint endpoint = m_remoteEndpoint;

        m_request.setSize(length);
        m_response.reset();

        uint16_t idx_request = 1;
        
        if(length == wordSize * 16 || m_request[0] == m_statusRequest.header)
        {
            sendStatusResponse(endpoint);
            return;
        }
        
        memory->lock();
        
        for(uint16_t idx = 0; idx < m_request.getTransactionNumber(); idx++)
        {
            TransactionHeader* headerRequest = (TransactionHeader*) m_request.getBuffer() + (idx_request++);
            if(headerRequest->protocolVersion != 2)
            {
                response.addTransaction(headerRequest->type, nullptr, 1, BadHeader);
            }

            switch(headerRequest->type)
            {
                case DataRead:
                {
                    InfoCode code = read(m_request[idx_request], headerRequest->words, m_buffer);
                    m_response.addTransaction(headerRequest->type, m_buffer, headerRequest->words, code);
                    idx_request += 1;
                }
                break;
                case NonIncrementingRead:
                {
                    InfoCode code = readNonIncrement(m_request[idx_request], headerRequest->words, m_buffer);
                    m_response.addTransaction(headerRequest->type, m_buffer, headerRequest->words, code);
                    idx_request += 1;
                }
                break;
                case DataWrite:
                {
                    InfoCode code = write(m_request[idx_request], headerRequest->words, m_request + idx_request + 1);
                    m_response.addTransaction(headerRequest->type, m_buffer, headerRequest->words, code);
                    idx_request += headerRequest->words + 1;
                }
                break;
                case NonIncrementingWrite:
                {
                    InfoCode code = writeNonIncrement(m_request[idx_request], headerRequest->words, m_request + idx_request + 1);
                    m_response.addTransaction(headerRequest->type, m_buffer, headerRequest->words, code);
                    idx_request += headerRequest->words + 1;
                }
                case RMWbits:
                {
                    InfoCode code = RMWbits(m_request[idx_request], m_request[idx_request+1], m_request[idx_request+2]);
                    response.addTransaction(headerRequest->type, m_buffer, 1, code);
                    idx_request += 3;
                }
                break;
                case RMWSum:
                {
                    InfoCode code = RMWsum(m_request[idx_request], m_request[idx_request+1]);
                    response.addTransaction(headerRequest->type, m_buffer, 1, code);
                    idx_request += 2;
                }
                break;

                default:
                {
                    response.addTransaction(headerRequest->type, nullptr, 1, ErrorRead);
                }
                break;
            }
        }
        memory->unlock();

        sendResponse(endpoint);
    }

    void IPbusSlave::sendResponse(const boost::asio::ip::udp::endpoint& endpoint)
    {
        m_socket.send_to(boost::asio::buffer(m_response.getBuffer(), m_response.getSize()*wordSize), endpoint);
    }

    void sendStatusResponse(const boost::asio::ip::udp::endpoint& endpoint)
    {
        m_socket.send_to(boost::asio::buffer(m_statusResponse, 16*wordSize), endpoint);
    }

    InfoCode IPbusSlave::read(uint32_t address, uint8_t words, uint32_t* out)
    {
        if(memory->data_read(address, words, out))
        {
            return Response;
        }
        else return ErrorRead;
    }

    InfoCode IPbusSlave::write(uint32_t address, uint8_t words, uint32_t* in)
    {
        if(memory->data_write(address, words, in))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::RMWbits(uint32_t address, uint32_t and, uint32_t or)
    {
        if(memory->data_read(address, 1, m_buffer) == false)
        {
            return ErrorRead;
        }
        m_buffer[1] = m_buffer[0];
        m_buffer[1] = (m_buffer[1] & and) | or;
        if(memory->data_write(address, 1, m_buffer + 1))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::RMWsum(uint32_t address, uint32_t add)
    {
        if(memory->data_read(address, 1, m_buffer) == false)
        {
            return ErrorRead;
        }
        m_buffer[1] = m_buffer[0];
        m_buffer[1] = m_buffer[1] + add;
        if(memory->data_write(address, 1, m_buffer + 1))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::readNonIncrement(uint32_t address, uint8_t words, uint32_t* out)
    {
        for(int i = 0; i < words; i++)
        {
            if(memory->data_read(address, 1, out + i) == false)
            {
                return ErrorRead;
            }
        }
        return Response;
    }

    InfoCode IPbusSlave::writeNonIncrement(uint32_t address, uint8_t words, uint32_t* in)
    {
        for(int i = 0; i < words; i++)
        {
            if(memory->data_write(address, 1, in + i) == false)
            {
                return ErrorWrite;
            }
        }
        return Response;
    }
} // namespace ipbus

