#ifndef SWTELECTRONICS_H
#define SWTELECTRONICS_H

#include "IPbusMaster.h"
#include"IPbusRequest.h"
#include"IPbusResponse.h"
#include "Swt.h"
#include "dimrpcparallel.h"
#include <string>
#include <boost/asio.hpp>
#include <map>
#include <boost/log/trivial.hpp>

namespace fit_swt
{

class SwtLink : public ipbus::IPbusMaster, DimRpcParallel
{
 public:
  SwtLink(std::string rpc, boost::asio::io_context& ioContext, std::string address = "172.20.75.175", uint16_t rport = 50001, uint16_t lport = 0) : IPbusMaster(ioContext, address, lport, rport),
                                                                                                                                                    DimRpcParallel(rpc.c_str(), "C", "C", 0)
  {
    BOOST_LOG_TRIVIAL(info) << "SWT-IPbus link initialization - " << address << ":" << rport;
  }

  void rpcHandler();
  void processRequest(const char* swtSequence);
  void writeFrame(Swt frame);
  void sendFailure();

  void splitLines(const char* swtSequence);
  bool parseFrames();
  bool interpretFrames();

  void writeToResponse();
  void sendResponse();

  void setPacketPadding(int);
  int getPacketPadding() const;

 private:
  ipbus::IPbusRequest m_packet;
  ipbus::IPbusResponse m_ipbusResponse;
  int m_lineBeg{0};
  int m_lineEnd{0};
  int m_packetPadding{8};

  std::vector<std::string> m_lines;
  std::vector<Swt> m_frames;
  std::string m_response;
};

} // namespace fit_swt

#endif // SWTELECTRONICS_H
