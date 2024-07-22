#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include"IPbusInterface.h"
#include"Swt.h"
#include"dimrpcparallel.h"
#include<string>
#include<boost/asio.hpp>

class SwtElectronics: public IPbusTarget, DimRpcParallel
{
public:
    SwtElectronics(std::string rpc, boost::asio::io_context & ioContext, std::string address = "172.20.75.175", uint16_t rport=50001, uint16_t lport=0): 
    IPbusTarget(ioContext, address, lport, rport),
    DimRpcParallel(rpc.c_str(), "C", "C", 0)
    {

    }

    void rpcHandler();
    void processRequest(const char* swtSequence);
    void writeFrame(Swt frame);

    void splitLines(const char* swtSequence);
    void parseFrames();
    void execute();

private:
    std::vector<std::string> m_lines;
    std::vector<Swt> m_frames;
    std::string m_response;
};

#endif // SWTELECTRONICS_H
