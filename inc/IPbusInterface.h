#ifndef IPbusInterface_H
#define IPbusInterface_H

#include <mutex>
#include <boost/asio.hpp>
#include <memory>
#include <pthread.h>

#include "IPbusControlPacket.h"

/* Should be used instead of simple "return" in every method that locks m_link_mutex */
#define RETURN_AND_RELEASE(mutex, statement) \
  pthread_mutex_unlock(&mutex);              \
  return statement;

namespace ipbus
{

class IPbusTarget
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
  timeval m_timeout{2, 0};

  pthread_mutex_t m_linkMutex;
  void intializeMutex(pthread_mutex_t& mutex);

 public:
  IPbusTarget(boost::asio::io_context& ioContext, std::string address = "172.20.75.175", uint16_t lport = 0, uint16_t rport = 50001);
  virtual ~IPbusTarget() {}

  bool checkStatus();
  bool reopen();

  bool transceive(IPbusControlPacket& p, bool shouldResponseBeProcessed = true);

  bool isIPbusOK() { return m_isAvailable; }

  void setTimeout(timeval timeout);
  timeval getTimeout() const;
};

} // namespace ipbus

#endif