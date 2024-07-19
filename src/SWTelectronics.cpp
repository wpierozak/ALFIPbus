#include"SWTelectronics.h"
#include<string>
void SWTelectronics::rpcHandler() 
{
    process_request(getString());
}

void SWTelectronics::process_request(const char* swt_sequence)
{
    std::string message = swt_sequence;
    std::cerr << message << std::endl;
    message = message.substr(message.find("0x")+2);
    int size = message.size();
    for(int i = message.find(','); i < size; i++)
        message.pop_back();

    message = "300000000000" + message;
    std::cout<< "swt message: " << message << std::endl;

    SWT frame = string_to_swt(message.c_str());
    SWT_IPBUS_READY rframe = swt_ready(frame);
    m_packet.addTransaction((rframe.type == SWT_IPBUS_READY::Type::Read) ? TransactionType::ipread : TransactionType::ipwrite,
                                    rframe.address, &rframe.data, 1);

    if(transcieve(m_packet))
    {
        write_response(frame, rframe);
    }
    else
    {
        setData("failure");
    }
}

void SWTelectronics::write_response(SWT frame, SWT_IPBUS_READY rframe)
{
    frame.data = rframe.data;

    uint8_t buffer[10];
    swt_to_byte(frame, buffer);

    m_response = "success 0\n0x00000000000";

    for(int i = 3; i >= 0; i--)
    {
        m_response += hexToChar(buffer[i] >> 4);
        m_response += hexToChar(buffer[i] & 0x0F);
    }
    
    //m_response += "\n";
    setData(m_response.c_str());
}