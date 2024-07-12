#ifndef IPBUSSWT_H
#define IPBUSSWT_H
#include"SWT.h"
#include"IPbusControlPacket.h"

class IPbusSWT_Packet
{
public:
    IPbusSWT_Packet()
    {

    }

    void add_transaction(const quint8* bytes)
    {
        swt_req.push_back(byte_to_swt(bytes));
        rswt.push_back(swt_ready(swt_req.back()));
        if(rswt.back().type == SWT_IPBUS_READY::Type::Read)
            packet.addTransaction(TransactionType::ipread, rswt.back().address, &rswt.back().data, 1);
        else
            packet.addTransaction(TransactionType::ipwrite, rswt.back().address, &rswt.back().data, 1);
    }

    void swt_response(SWT frame, SWT_IPBUS_READY r_frame)
    {
        if(r_frame.type == SWT_IPBUS_READY::Type::Write)
            swt_res.push_back(frame);
        frame.data = r_frame.data;
        swt_res.push_back(frame);
    }

    void translate_response(int transaction_id)
    {
        swt_response(swt_req[transaction_id], rswt[transaction_id]);
    }

    QList<SWT> swt_res;
    QList<SWT> swt_req;
    QList<SWT_IPBUS_READY> rswt;
    IPbusControlPacket packet;
};

#endif // IPBUSSWT_H
