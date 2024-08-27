#ifndef IPbusInterface_H
#define IPbusInterface_H

#include <mutex>
#include <boost/asio.hpp>
#include <memory>
#include<chrono>
#include <pthread.h>

#include "IPbusPacket.h"
#include "IPbusRequest.h"
#include "IPbusResponse.h"
#include "IPbusStatusPacket.h"

/* Should be used instead of simple "return" in every method that locks m_link_mutex */
#define RETURN_AND_RELEASE(mutex, statement) \
  pthread_mutex_unlock(&mutex);              \
  return statement;

namespace ipbus
{

class IPbusMaster
{
 private:
  // BOOST ASIO //

  boost::asio::io_context& m_ioContext;

  uint16_t m_localPort;
  uint16_t m_remotePort;

  std::string m_ipAddress;

  boost::asio::ip::udp::endpoint m_localEndpoint;
  boost::asio::ip::udp::endpoint m_remoteEndpoint;

  boost::asio::ip::udp::socket m_socket;

  // IPBUS status //

  const StatusPacket m_status;
  StatusPacket m_statusRespone;
  bool m_isAvailable{ false };

  bool openSocket();

  // Sync communication

  size_t receive(char* destBuffer, size_t maxSize);

  void handleReceive(const boost::system::error_code& ec, std::size_t length);
  void handleDeadline();
  boost::asio::deadline_timer m_timer;

  enum class ReceiveStatus{Wait, Expired, Received} m_receiveStatus;
  pthread_mutex_t m_receiveStatusMutex;

  size_t m_receivedSize;
  boost::system::error_code m_error;

  boost::posix_time::milliseconds m_timeout{2000};

  pthread_mutex_t m_linkMutex;
  void intializeMutex(pthread_mutex_t& mutex);

  bool processResponse(IPbusRequest& request, IPbusResponse& response);

 public:

  IPbusMaster(boost::asio::io_context& ioContext, std::string address = "172.20.75.175", uint16_t lport = 0, uint16_t rport = 50001);
  virtual ~IPbusMaster() {}

  bool checkStatus();
  bool reopen();

  bool transceive(IPbusRequest& request, IPbusResponse& response);

  bool isIPbusOK() { return m_isAvailable; }

  void setTimeout(boost::posix_time::milliseconds timeout);
  boost::posix_time::milliseconds getTimeout() const;
};

} // namespace ipbus

#endif