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
    runStatusThread();
    BOOST_LOG_TRIVIAL(info) << "SWT-IPbus link initialized - " << address << ":" << rport;
  }

  void processRequest(const char* swtSequence);
  void rpcHandler();
  void sendFailure();

  bool interpretFrames();
  bool executeTransactions();
  void resetState();

  bool readBlock(const Swt& frame);

  void processExecutedCommands();
  void sendResponse();

  void terminate() { m_terminate = true; }

 private:
  void updateFifoState(const Swt& frame);
  void runStatusThread();

  CruCommand& parseNextCommand(const char* &currentLine);

  bool parseSequence(const char* request);

  // Inlines
  
  inline bool isIPbusPacketFull(){
    return (m_request.getSize() + PacketPadding >= ipbus::maxPacket);
  }

  inline bool validateRmwTransaction(const Swt& frame, const bool& expectRmwOr)
  {
    if(expectRmwOr == false){
        BOOST_LOG_TRIVIAL(error) << "Received RMW OR before RMW AND!";
        return false;
    } else if(frame.address != m_commands[m_cmdFifoSize-2].frame.address){
        BOOST_LOG_TRIVIAL(error) << "RMW OR address mismatch!";
        return false;
    }
    return true;
  }

  ipbus::IPbusRequest m_request;
  ipbus::IPbusResponse m_response;

  int m_current{0};
  int m_lastWritten{0};
  
  static constexpr uint32_t PacketPadding = 4;
  
  std::unique_ptr<std::thread> m_checkStatusParallel;

  std::array<CruCommand,1024> m_commands;
  uint32_t m_cmdFifoSize{0};
  std::string m_fredResponse;
  SwtFifo m_fifo;

  bool m_terminate {false};
};

} // namespace fit_swt

#endif // SWTELECTRONICS_H
