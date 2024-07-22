#include"SWTelectronics.h"
#include"utils.h"
#include<string>

void SWTelectronics::rpcHandler() 
{
    process_request(getString());
}

void SWTelectronics::process_request(const char* swt_sequence)
{
    split_lines(swt_sequence);
    parse_frames();
    execute();
}

void SWTelectronics::split_lines(const char* swt_sequence)
{
    std::string swt_str = swt_sequence;
    m_lines = utils::splitString(swt_str, "\n");
}

void SWTelectronics::parse_frames()
{
    m_frames.clear();
    m_lines.erase(m_lines.begin());

    for(auto frame : m_lines)
    {
        if(frame.find("write") == std::string::npos) continue;

        frame = frame.substr(frame.find("0x")+2);
        int size = frame.size();
        for(int i = frame.find(','); i < size; i++)
            frame.pop_back();

        try
        {
            m_frames.emplace_back(string_to_swt(frame.c_str()));
            TransactionType type = (m_frames.back().getTransactionType() == SWT::TransactionType::READ) ? data_read : data_write;
            m_packet.addTransaction(type, m_frames.back().address, &m_frames.back().data, 1);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }   
}

void SWTelectronics::execute()
{
    if(transcieve(m_packet))
    {
        m_response = "success ";
        for(int i = 0; i < m_lines.size(); i++)
        {
            if(m_lines[i] == "read")
            {
                write_frame(m_frames[i-1]);
                m_response += "\n";
                continue;
            }
            else if(m_lines[i].find("write") != std::string::npos)
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

void SWTelectronics::write_frame(SWT frame)
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