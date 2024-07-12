#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include"IPbusInterface.h"
#include"ipbusswt.h"

class SWTelectronics: public IPbusTarget
{
public:
    SWTelectronics(QString address = "172.20.75.180", quint16 lport = 0): IPbusTarget(address,lport)
    {

    }

    void process_request(const quint8* swt_sequence)
    {
        m_packet.add_transaction(swt_sequence);
        if(transceive(m_packet.packet))
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

public slots:
    void sync() override
    {

    }

};

#endif // SWTELECTRONICS_H
