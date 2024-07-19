#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include"IPbusInterface.h"
#include"SWT.h"
#include"dimrpcparallel.h"
#include<string>
#include<boost/asio.hpp>

class SWTelectronics: public IPbusTarget, DimRpcParallel
{
public:
    SWTelectronics(std::string rpc, boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t rport=50001, uint16_t lport=0): 
    IPbusTarget(io_context, address, lport, rport),
    DimRpcParallel(rpc.c_str(), "C", "C", 0)
    {

    }

    void rpcHandler();
    void process_request(const char* swt_sequence);

    void write_response(SWT frame, SWT_IPBUS_READY rframe);

private:
    std::string m_response;
};

#endif // SWTELECTRONICS_H
