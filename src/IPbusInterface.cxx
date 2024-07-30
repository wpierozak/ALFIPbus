#include <boost/bind.hpp>
#include <iostream>
#include "IPbusInterface.h"
#include <boost/log/trivial.hpp>
#include <sys/socket.h>

namespace ipbus
{

IPbusTarget::IPbusTarget(boost::asio::io_context& ioContext, std::string address, uint16_t lport, uint16_t rport) : m_ioContext(ioContext),
                                                                                                                    m_localPort(lport),
                                                                                                                    m_remotePort(rport),
                                                                                                                    m_ipAddress(address),
                                                                                                                    m_localEndpoint(boost::asio::ip::udp::v4(), m_localPort),
                                                                                                                    m_remoteEndpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_ipAddress), m_remotePort)),
                                                                                                                    m_timer(ioContext),
                                                                                                                    m_socket(ioContext)

{
  openSocket();
  intializeMutex(m_linkMutex);
  intializeMutex(m_receiveStatusMutex);
  checkStatus();
}

bool IPbusTarget::openSocket()
{
  BOOST_LOG_TRIVIAL(debug) << "Attempting to open socket...";
  m_socket.open(boost::asio::ip::udp::v4());

  if (m_socket.is_open()) {
    BOOST_LOG_TRIVIAL(info) << "Socket at " << m_ipAddress << ":" << m_remotePort << " successfully opened";
    return true;
  } else {
    BOOST_LOG_TRIVIAL(error) << "Failed to open socket at " << m_ipAddress << ":" << m_localPort;
    return false;
  }
}

bool IPbusTarget::reopen()
{
  if (m_socket.is_open()) {
    BOOST_LOG_TRIVIAL(debug) << "Socket is already open.";
    return true;
  }
  BOOST_LOG_TRIVIAL(debug) << "Socket is not open. Attempting to reopen...";
  return openSocket(); // Reopen the socket if it is not open
}

size_t IPbusTarget::receive(char* destBuffer, size_t maxSize)
{
  try {
    BOOST_LOG_TRIVIAL(debug) << "Synchronous receiving...";
    m_socket.async_receive_from(boost::asio::buffer(destBuffer, maxSize), m_remoteEndpoint, boost::bind(&IPbusTarget::handleReceive, this, std::placeholders::_1, std::placeholders::_2));
    
    m_receivedSize = 0;
    m_receiveStatus = ReceiveStatus::Wait;
    
    m_timer.expires_from_now(m_timeout);
    m_timer.async_wait(boost::bind(&IPbusTarget::handleDeadline, this));

    do
    {
      m_ioContext.run_one();
    } 
    while(m_error == boost::asio::error::would_block || m_receiveStatus == ReceiveStatus::Wait);

    return m_receivedSize;

  } catch (const boost::system::system_error& e) {
    BOOST_LOG_TRIVIAL(error) << "Receive error: " << e.what();
    return 0;
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Error: " << e.what();
    return 0;
  } catch (...) {
    BOOST_LOG_TRIVIAL(error) << "An unknown error occurred during receive operation.";
    return 0;
  }
  return 0;
}

void IPbusTarget::handleReceive(const boost::system::error_code& ec, std::size_t length)
{
  pthread_mutex_lock(&m_receiveStatusMutex);

  BOOST_LOG_TRIVIAL(debug) << "Message received: " << length << " bytes";

  m_receiveStatus = ReceiveStatus::Received;
  m_error = ec;
  m_receivedSize = length;

  m_timer.cancel();

  pthread_mutex_unlock(&m_receiveStatusMutex);
}

void IPbusTarget::handleDeadline()
{
  if(m_timer.expires_at() > boost::asio::deadline_timer::traits_type::now())
  {
    m_timer.async_wait(boost::bind(&IPbusTarget::handleDeadline, this));
    return;
  }

  pthread_mutex_lock(&m_receiveStatusMutex);

  if(m_receiveStatus == ReceiveStatus::Wait)
  {
    BOOST_LOG_TRIVIAL(error) << "Request has timed out";

    m_receiveStatus = ReceiveStatus::Expired;
    m_socket.cancel();
    m_timer.expires_at(boost::posix_time::pos_infin);
  }

  pthread_mutex_unlock(&m_receiveStatusMutex);
}

bool IPbusTarget::checkStatus()
{
  pthread_mutex_lock(&m_linkMutex);
  bool response = true;

  try {
    BOOST_LOG_TRIVIAL(debug) << "Checking status of device at " << m_ipAddress << ":" << m_remotePort;
    
    m_isAvailable = false;

    m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remoteEndpoint);
    size_t bytes = receive((char*)&m_statusRespone, sizeof(m_statusRespone));

    if(m_error)
    {
      BOOST_LOG_TRIVIAL(info) << m_error.message();
    }
    else if(m_receiveStatus == ReceiveStatus::Expired)
    {
      BOOST_LOG_TRIVIAL(info) << "Status packet has not been received";
    }
    else if(bytes < sizeof(m_status))
    {
       BOOST_LOG_TRIVIAL(info) << "Status packet is too short - received " << bytes << " (expected " << sizeof(m_status) << ")";
    }
    else
    {
      BOOST_LOG_TRIVIAL(info) << "Status check successful: Device at " << m_ipAddress << ":" << m_remotePort << " is available.";
      m_isAvailable = true;
    }

    response = true;

  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to check status of device at " << m_ipAddress << ":" << m_remotePort << ": " << e.what() << std::endl;
    m_isAvailable = false;

    response = false;
  }

  RETURN_AND_RELEASE(m_linkMutex, response);
}

bool IPbusTarget::transceive(IPbusControlPacket& p, bool shouldResponseBeProcessed)
{
  pthread_mutex_lock(&m_linkMutex);

  if (m_isAvailable == false) {
    BOOST_LOG_TRIVIAL(error) << "Device at " << m_ipAddress << ":" << m_remotePort << " is not available";

    checkStatus();
    if (m_isAvailable == false){
      RETURN_AND_RELEASE(m_linkMutex, false);
    }
  }
  
  if (p.m_requestSize <= 1) {
    BOOST_LOG_TRIVIAL(warning) << "Empty request";
    RETURN_AND_RELEASE(m_linkMutex, true);
  }

  size_t send_bytes = 0;

  try {
    send_bytes = m_socket.send_to(boost::asio::buffer((char*)&p.m_request, p.m_requestSize * wordSize), m_remoteEndpoint);
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Sending packet to " << m_ipAddress << ":" << m_remotePort << " failed: " << e.what();
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  if (send_bytes < p.m_requestSize * wordSize) {
    BOOST_LOG_TRIVIAL(error) << "Sending packet to " << m_ipAddress << ":" << m_remotePort << " failed: " << send_bytes << " bytes were sent instead of " << p.m_requestSize * wordSize;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  size_t bytes_recevied = receive((char*)&p.m_response, maxPacket*wordSize);

  if(m_receiveStatus == ReceiveStatus::Expired)
  {
    checkStatus();
    if(m_isAvailable == false)
    {
      RETURN_AND_RELEASE(m_linkMutex, false);
    }
    else
    {
      BOOST_LOG_TRIVIAL(error) << "Device is available but request has expired - request failed";
      RETURN_AND_RELEASE(m_linkMutex, false);
    }
  }

  if (bytes_recevied == 64 && p.m_response[0] == m_status.header) {
    bytes_recevied = receive((char*)&p.m_response, maxPacket*wordSize);
  }

  if (bytes_recevied == 0) {
    BOOST_LOG_TRIVIAL(error) << "Empty response from " << m_ipAddress << ":" << m_remotePort;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else if (bytes_recevied / wordSize > p.m_responseSize || p.m_response[0] != p.m_request[0] || bytes_recevied % wordSize > 0) {
    BOOST_LOG_TRIVIAL(error) << "Incorrect response from " << m_ipAddress << ":" << m_remotePort << ": received " << bytes_recevied << " bytes instead of " << p.m_responseSize * wordSize;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else {
    p.m_responseSize = uint16_t(bytes_recevied / wordSize); // response can be shorter then expected if a transaction wasn't successful
    if (shouldResponseBeProcessed) {
      BOOST_LOG_TRIVIAL(debug) << "Processing response";
    } else {
      BOOST_LOG_TRIVIAL(debug) << "Response will not be processed";
    }
    bool result = shouldResponseBeProcessed ? p.processResponse() : true;
    if(!result)
      BOOST_LOG_TRIVIAL(error) << "Failed to process response";
    p.reset();
    RETURN_AND_RELEASE(m_linkMutex, result);
  }
  RETURN_AND_RELEASE(m_linkMutex, false);
}

void IPbusTarget::intializeMutex(pthread_mutex_t& mutex)
{
  pthread_mutex_init(&mutex, NULL);
}

void IPbusTarget::setTimeout(boost::posix_time::milliseconds timeout)
{
  m_timeout = timeout;
}

boost::posix_time::milliseconds IPbusTarget::getTimeout() const
{
  return m_timeout;
}

} // namespace ipbus