#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include "IPbus/IPbusMaster.h"
#include "IPbus/IPbusRequest.h"
#include "IPbus/IPbusResponse.h"
#include "Swt.h"
#include "dimrpcparallel.h"
#include <string>
#include <boost/asio.hpp>
#include <map>
#include <boost/log/trivial.hpp>
#include"utils.h"
#include"CruCommand.h"
#include"SwtFifo.h"
#include<span>
namespace fit_swt
{

class SwtLink : public ipbus::IPbusMaster, DimRpcParallel
{
 public:
  SwtLink(std::string rpc, boost::asio::io_context& ioContext, std::string address = "172.20.75.175", uint16_t rport = 50001, uint16_t lport = 0) : IPbusMaster(ioContext, address, lport, rport),
                                                                                                                                                    DimRpcParallel(rpc.c_str(), "C", "C", 0)
  {
    if(isIPbusOK() == false)
    {
      for(int i = 0; i < 4; i++)
      {
        checkStatus();
        if(isIPbusOK() == true) break;
      }
    }

    BOOST_LOG_TRIVIAL(info) << "SWT-IPbus link initialization - " << address << ":" << rport;
  }

  void rpcHandler();
  void processRequest(const char* swtSequence);
  void writeFrame(const Swt& frame);
  void sendFailure();

  bool parseFrames(const char* request);
  bool interpretFrames();
  bool executeTransactions();
  void resetState();

  bool readBlock(const Swt& frame, uint32_t frameIdx);

  void processExecutedCommands();
  void sendResponse();

 private:
  void updateFifoState(const Swt& frame);
  void updateFifoState(const Swt& frame, std::span<Swt> response);

  ipbus::IPbusRequest m_request;
  ipbus::IPbusResponse m_response;

  int m_current{0};
  int m_lastWritten{0};
  
  static constexpr uint32_t PacketPadding = 4;
  
  std::vector<CruCommand> m_commands;
  std::string m_fredResponse;
  
  size_t m_sequenceLen;
  uint32_t m_commandsNumber;
  SwtFifo m_fifo;
};

} // namespace fit_swt

#endif // SWTELECTRONICS_H
