#ifndef IPBUSSWT_H
#define IPBUSSWT_H
#include"IPbusInterface.h"
#include"SWT.h"

class IPbusSWT: public IPbusTarget
{
public:
    IPbusSWT(QString address, quint16 lport): IPbusTarget(lport)
    {
        IPaddress = address;
        reconnect();
    }

    void add_transaction(const char* str)
    {
        frames.push_back(string_to_swt(str));
        r_frames.push_back(swt_ready(frames.back()));
        if(r_frames.back().type == SWT_IPBUS_READY::Type::Read)
            packet.addTransaction(TransactionType::ipread, r_frames.back().address, &r_frames.back().data, 1);
        else
            packet.addTransaction(TransactionType::ipwrite, r_frames.back().address, &r_frames.back().data, 1);
    }

    void swt_response(SWT frame, SWT_IPBUS_READY r_frame)
    {
        if(r_frame.type == SWT_IPBUS_READY::Type::Write)
            responses.push_back(frame);
        frame.data = r_frame.data;
        responses.push_back(frame);
    }

    void sync() final
    {
        reconnect();
    }

    QList<SWT> responses;
    QList<SWT> frames;
    QList<SWT_IPBUS_READY> r_frames;
    IPbusControlPacket packet;
};

#endif // IPBUSSWT_H
