#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include"IPbusInterface.h"
#include"IPbusPacket_SWT.h"
#include<string>
#include<boost/asio.hpp>

class SWTelectronics: public IPbusTarget
{
public:
    SWTelectronics(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=0,
 uint16_t rport=50001): IPbusTarget(io_context, address, lport, rport)
    {

    }

    void process_request(const uint8_t* swt_sequence)
    {
        m_packet.add_transaction(swt_sequence);
        if(transcieve(m_packet.packet))
        {
            m_packet.translate_response(0);
            write_response(m_packet.swt_res[0]);
        }
    }

    void write_response(SWT word)
    {

    }

private:
    IPbusSWT_Packet m_packet;
};

#endif // SWTELECTRONICS_H
