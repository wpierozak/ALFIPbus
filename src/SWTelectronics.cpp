#include"SWTelectronics.h"
#include"Utils.h"
#include<string>
void SWTelectronics::rpcHandler() 
{
    process_request(getString());
}

void SWTelectronics::process_request(const char* swt_sequence)
{
    std::string swt_str = swt_sequence;
    std::vector<std::string> lines = Utils::splitString(swt_str, "\n");
    std::vector<std::pair<SWT, SWT_IPBUS_READY>> swt_frames;

    lines.erase(lines.begin());
    lines.pop_back();

    for(auto frame : lines)
    {
        std::cerr << frame << std::endl;

        frame = frame.substr(frame.find("0x")+2);
        int size = frame.size();
        for(int i = frame.find(','); i < size; i++)
            frame.pop_back();
        
        std::cerr << frame << std::endl;

        SWT swt = string_to_swt(frame.c_str());
        swt_frames.emplace_back(swt, swt_ready(swt));

        m_packet.addTransaction(
            (swt_frames.back().second.type == SWT_IPBUS_READY::Type::Read) ? TransactionType::data_read : TransactionType::data_write,
             swt_frames.back().second.address, &swt_frames.back().second.data, 1);
    }

    if(transcieve(m_packet))
    {
        for(int i = 0; i < swt_frames.size(); i++)
        {
            write_response(swt_frames[i].first, swt_frames[i].second);
            if( i != swt_frames.size() - 1)
            {
                m_response += "\n";
            }
        }
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