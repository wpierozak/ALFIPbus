#include"IPbusSlave.h"
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

namespace ipbus
{
    IPbusSlave::IPbusSlave(boost::asio::io_context& io, Memory* m_memory, uint16_t lport):
        m_ioContext(io),
        m_memory(m_memory),
        m_socket(io),
        m_localPort(lport),
        m_localEndpoint(boost::asio::ip::udp::v4(), lport)
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
            m_socket.bind(m_localEndpoint);
            BOOST_LOG_TRIVIAL(info) << "Socket at " << m_localPort << " successfully opened";
            return true;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Failed to open socket at ";
        }
    }

    bool IPbusSlave::startAsyncRecv()
    {
        BOOST_LOG_TRIVIAL(info) << "Initalizing async receiving";
        m_socket.async_receive_from(boost::asio::buffer((char*) m_request.getBuffer(), maxPacket*wordSize), m_remoteEndpoint,
                                boost::bind(&IPbusSlave::handleRequest, this, boost::placeholders::_1, boost::placeholders::_2));
    }

    void IPbusSlave::handleRequest(const boost::system::error_code& ec, std::size_t length)
    {
        
        BOOST_LOG_TRIVIAL(info) << "Processing...";
        boost::asio::ip::udp::endpoint endpoint = m_remoteEndpoint;

        m_request.setSize(length/wordSize);
        m_response.reset();

        uint16_t idx_request = 1;
        
        if(length == wordSize * 16 && m_request[0] == m_statusRequest.header)
        {
            sendStatusResponse(endpoint);
            startAsyncRecv();
            return;
        }
        
        m_memory->lock();
        

        while(idx_request < m_request.getSize())
        {
            TransactionHeader* headerRequest = (TransactionHeader*) m_request.getBuffer() + (idx_request++);
            if(headerRequest->protocolVersion != 2)
            {
                m_response.addTransaction((TransactionType) headerRequest->typeID, nullptr, 1, BadHeader);
            }

            BOOST_LOG_TRIVIAL(info) << headerRequest->typeID;

            switch(headerRequest->typeID)
            {
                case DataRead:
                {
                    InfoCode code = read(m_request[idx_request], headerRequest->words, m_buffer);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, m_buffer, headerRequest->words, code);
                    idx_request += 1;
                }
                break;
                case NonIncrementingRead:
                {
                    InfoCode code = readNonIncrement(m_request[idx_request], headerRequest->words, m_buffer);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, m_buffer, headerRequest->words, code);
                    idx_request += 1;
                }
                break;
                case DataWrite:
                {
                    InfoCode code = write(m_request[idx_request], headerRequest->words, m_request.getBuffer() + idx_request + 1);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, nullptr, 0, code);
                    idx_request += headerRequest->words + 1;
                }
                break;
                case NonIncrementingWrite:
                {
                    InfoCode code = writeNonIncrement(m_request[idx_request], headerRequest->words, m_request.getBuffer() + idx_request + 1);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, nullptr, 0, code);
                    idx_request += headerRequest->words + 1;
                }
                break;
                case RMWbits:
                {
                    InfoCode code = rmwBits(m_request[idx_request], m_request[idx_request+1], m_request[idx_request+2]);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, m_buffer, 1, code);
                    idx_request += 3;
                }
                break;
                case RMWsum:
                {
                    InfoCode code = rmwSum(m_request[idx_request], m_request[idx_request+1]);
                    m_response.addTransaction((TransactionType)headerRequest->typeID, m_buffer, 1, code);
                    idx_request += 2;
                }
                break;

                default:
                {
                    m_response.addTransaction((TransactionType)headerRequest->typeID, nullptr, 1, ErrorRead);
                }
                break;
            }
        }
        m_memory->unlock();

        sendResponse(endpoint);
        startAsyncRecv();
    }

    void IPbusSlave::sendResponse(const boost::asio::ip::udp::endpoint& endpoint)
    {
        m_socket.send_to(boost::asio::buffer((char*)m_response.getBuffer(), m_response.getSize()*wordSize), endpoint);
        m_request.reset();
    }

    void IPbusSlave::sendStatusResponse(const boost::asio::ip::udp::endpoint& endpoint)
    {
        m_socket.send_to(boost::asio::buffer((char*)&m_statusResponse, 16*wordSize), endpoint);
    }

    InfoCode IPbusSlave::read(uint32_t address, uint8_t words, uint32_t* out)
    {
        if(m_memory->dataRead(address, words, out))
        {
            return Response;
        }
        else return ErrorRead;
    }

    InfoCode IPbusSlave::write(uint32_t address, uint8_t words, uint32_t* in)
    {
        if(m_memory->dataWrite(address, words, in))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::rmwBits(uint32_t address, uint32_t andMask, uint32_t orMask)
    {
        if(m_memory->dataRead(address, 1, m_buffer) == false)
        {
            return ErrorRead;
        }
        m_buffer[1] = m_buffer[0];
        m_buffer[1] = (m_buffer[1] & andMask) | orMask;
        if(m_memory->dataWrite(address, 1, m_buffer + 1))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::rmwSum(uint32_t address, uint32_t add)
    {
        if(m_memory->dataRead(address, 1, m_buffer) == false)
        {
            return ErrorRead;
        }
        m_buffer[1] = m_buffer[0];
        m_buffer[1] = m_buffer[1] + add;
        if(m_memory->dataWrite(address, 1, m_buffer + 1))
        {
            return Response;
        }
        else return ErrorWrite;
    }

    InfoCode IPbusSlave::readNonIncrement(uint32_t address, uint8_t words, uint32_t* out)
    {
        for(int i = 0; i < words; i++)
        {
            if(m_memory->dataRead(address, 1, out + i) == false)
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
            if(m_memory->dataWrite(address, 1, in + i) == false)
            {
                return ErrorWrite;
            }
        }
        return Response;
    }
} // namespace ipbus

