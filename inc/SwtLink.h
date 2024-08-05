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
  void writeFrame(Swt frame);
  void sendFailure();

  void splitLines(const char* swtSequence);
  bool parseFrames();
  bool interpretFrames();

  void writeToResponse();
  void sendResponse();

  void setPacketPadding(int);
  int getPacketPadding() const;

  static constexpr uint16_t EmptyMode = 0xFFFF;
  static constexpr uint16_t EmptyAddress = 0x0;
  static constexpr uint16_t EmptyData = 0x0;

 private:
  ipbus::IPbusRequest m_request;
  ipbus::IPbusResponse m_response;

  int m_lineBeg{0};
  int m_lineEnd{0};
  int m_packetPadding{8};

  std::vector<std::string> m_lines;
  std::vector<Swt> m_frames;
  std::string m_fredResponse;
};

} // namespace fit_swt

#endif // SWTELECTRONICS_H
