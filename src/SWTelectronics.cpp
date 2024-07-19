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

    std::cout<< "swt message: " << message << std::endl;

    SWT frame = string_to_swt(message.c_str());
    TransactionType type =  (frame.getTransactionType() == SWT::TransactionType::READ) ? data_read : data_write;
    
    m_packet.addTransaction(type, frame.address, &frame.data, 1);

    if(transcieve(m_packet))
    {
        write_response(frame);
    }
    else
    {
        setData("failure");
    }
}

void SWTelectronics::write_response(SWT frame)
{
    m_response = "success 0\n0x";
    half_word h; 
    h.data = frame.mode;

    std::string mode = half_word_to_string(h);
    mode = mode.substr(1);
    m_response += mode;

    word w; 
    w.data = frame.address;
    m_response += word_to_string(w);
    w.data = frame.data;
    m_response += word_to_string(w);
    
    setData(m_response.c_str());
}