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
    std::vector<SWT> swt_frames;

    lines.erase(lines.begin());

    for(auto frame : lines)
    {
        if(frame == "read") continue;

        frame = frame.substr(frame.find("0x")+2);
        int size = frame.size();
        for(int i = frame.find(','); i < size; i++)
            frame.pop_back();

        swt_frames.emplace_back(string_to_swt(frame.c_str()));
        TransactionType type = (swt_frames.back().getTransactionType() == SWT::TransactionType::READ) ? data_read : data_write;
        m_packet.addTransaction(type, swt_frames.back().address, &swt_frames.back().data, 1);
    }

    if(transcieve(m_packet))
    {
        m_response = "success ";
        for(int i = 0; i < lines.size(); i++)
        {
            if(lines[i] == "read")
            {
                write_response(swt_frames[i-1]);
                m_response += "\n";
                continue;
            }
            else if(lines[i].find("write") != std::string::npos)
            {
                m_response += "0\n";
            }
        }
        setData(m_response.c_str());
    }
    else
    {
        setData("failure");
    }
}

void SWTelectronics::write_response(SWT frame)
{
    m_response += "0x";
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
}