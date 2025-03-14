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
    m_checkStatusParallel = std::make_unique<std::thread>([&]{while(!isIPbusOK()) {checkStatus();}});
    m_checkStatusParallel->detach();
    BOOST_LOG_TRIVIAL(info) << "SWT-IPbus link initialization - " << address << ":" << rport;
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

 private:
  void updateFifoState(const Swt& frame);

  CruCommand& parseNextCommand(const char* &currentLine);
  std::string parseInvalidLine(const char* currentLine, const char*end);

  bool parseSequence(const char* request);
  bool isIPbusPacketFull(){
    return (m_request.getSize() + PacketPadding >= ipbus::maxPacket);
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
};

} // namespace fit_swt

#endif // SWTELECTRONICS_H
