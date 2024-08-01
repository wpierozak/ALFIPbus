#include"IPbusRequest.h"

namespace ipbus
{
    IPbusRequest::IPbusRequest()
    {
        m_buffer[0] = PacketHeader(Control, 0);
        m_size = 1;
        m_transactionsNumber = 0;
        m_expectedResponseSize = 0;
    }

    void IPbusRequest::reset()
    {
        m_size = 1;
        m_transactionsNumber = 0;
        m_expectedResponseSize = 0;
        m_dataOut.clear();
    }

    void IPbusRequest::addTransaction(TransactionType type, uint32_t address, uint32_t* dataIn, uint32_t* dataOut, uint8_t nWords)
    {
        if(m_size + nWords + 1 > maxPacket)
        {
            BOOST_LOG_TRIVIAL(error) << "Request packet size exceeded";
            return;
        }

        m_buffer[m_size++] = TransactionHeader(type, nWords, m_transactionsNumber++);
        m_buffer[m_size++] = address;
        m_expectedResponseSize++;

        switch (type)
        {
        case DataRead:
        case NonIncrementingRead:
        case ConfigurationRead:
            m_expectedResponseSize += nWords;
            break;
        case DataWrite:
        case NonIncrementingWrite:
        case ConfigurationWrite:
            for (uint8_t i = 0; i < nWords; ++i) 
            {
                m_buffer[m_size++] = dataIn[i];
            }
        case RMWbits:
            m_buffer[m_size++] = dataIn[0];
            m_buffer[m_size++] = dataIn[1];
            m_expectedResponseSize++;

        case RMWsum:
            m_buffer[m_size++] = dataIn[0];
            m_expectedResponseSize++;
        default:
            BOOST_LOG_TRIVIAL(warning) << "Unknown transaction type " << type << ": no transaction was added";
            break;
        }

        if(m_expectedResponseSize > maxPacket)
        {
            BOOST_LOG_TRIVIAL(error) << "Response packet size exceeded";
            // ADD THROW!!!
            return;
        }

        m_dataOut.push_back(dataOut);
    }
}