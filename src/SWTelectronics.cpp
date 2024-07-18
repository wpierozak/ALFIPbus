#include"SWTelectronics.h"

void SWTelectronics::rpcHandler() 
{
    process_request(getString());
}

void SWTelectronics::process_request(const char* swt_sequence)
{
    std::string message = "100000000000"; message += swt_sequence;
    SWT frame = string_to_swt(message.c_str());
    SWT_IPBUS_READY rframe = swt_ready(frame);
    m_packet.addTransaction((rframe.type == SWT_IPBUS_READY::Type::Read) ? TransactionType::ipread : TransactionType::ipwrite,
                                    rframe.address, &rframe.data, 1);

    if(transcieve(m_packet))
    {
        write_response(frame, rframe);
    }
}

void SWTelectronics::write_response(SWT frame, SWT_IPBUS_READY rframe)
{
    frame.data = rframe.data;

    uint8_t buffer[10];
    swt_to_byte(frame, buffer);

    m_response = "";

    for(int i = 3; i >= 0; i--)
    {
        m_response += hexToChar(buffer[i] >> 4);
        m_response += hexToChar(buffer[i] & 0x0F);
    }

    std::cout << m_response << std::endl;
    setData(m_response.c_str());
}