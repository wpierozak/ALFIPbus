#include <boost/bind.hpp>
#include <iostream>
#include "IPbusInterface.h"

namespace ipbus
{

IPbusTarget::IPbusTarget(boost::asio::io_context& ioContext, std::string address, uint16_t lport, uint16_t rport) : m_ioContext(ioContext),
                                                                                                                     m_localPort(lport),
                                                                                                                     m_remotePort(rport),
                                                                                                                     m_ipAddress(address),
                                                                                                                     m_localEndpoint(boost::asio::ip::udp::v4(), m_localPort),
                                                                                                                     m_remoteEndpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_ipAddress), m_remotePort)),
                                                                                                                     m_socket(ioContext)

{
  openSocket();
  intializeMutex(m_linkMutex);
  checkStatus();
}

bool IPbusTarget::openSocket()
{
  if (m_debug == DebugMode::Vital || m_debug == DebugMode::Full)
    std::cerr << "Attempting to open socket..." << std::endl;
  m_socket.open(boost::asio::ip::udp::v4());

  if (m_socket.is_open()) {
    if (m_debug == DebugMode::Vital || m_debug == DebugMode::Full)
      std::cerr << "Socket successfully opened" << std::endl;
    return true;
  } else {
    if (m_debug == DebugMode::Vital || m_debug == DebugMode::Full)
      std::cerr << "Failed to open socket." << std::endl;
    return false;
  }
}

bool IPbusTarget::reopen()
{
  if (m_socket.is_open()) {
    std::cerr << "Socket is already open." << std::endl;
    return true;
  }
  std::cerr << "Socket is not open. Attempting to reopen..." << std::endl;
  return openSocket(); // Reopen the socket if it is not open
}

size_t IPbusTarget::receive(char* destBuffer, size_t maxSize)
{
  try {
    if (m_debug == DebugMode::Full)
      std::cerr << "Synchronous receiving..." << std::endl;
    size_t bytesTransferred = m_socket.receive_from(boost::asio::buffer(destBuffer, IO_BUFFER_SIZE), m_remoteEndpoint);
    if (m_debug == DebugMode::Full)
      std::cerr << "Message received: " << bytesTransferred << " bytes" << std::endl;
    return bytesTransferred;
  } catch (const boost::system::system_error& e) {
    std::cerr << "Receive error: " << e.what() << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  } catch (...) {
    std::cerr << "An unknown error occurred during receive operation." << std::endl;
    return 0;
  }
  return 0;
}

bool IPbusTarget::checkStatus()
{
  pthread_mutex_lock(&m_linkMutex);
  bool response = true;

  try {
    if (m_debug == DebugMode::Vital | m_debug == DebugMode::Full)
      std::cerr << "Checking status of device at " << m_ipAddress << std::endl;
    // Send a status packet to the remote endpoint
    m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remoteEndpoint);
    receive((char*)&m_statusRespone, sizeof(m_statusRespone));
    if (m_debug == DebugMode::Vital || m_debug == DebugMode::Full)
      std::cerr << "Status check successful: Device is available." << std::endl;
    m_isAvailable = true;

    response = true;

  } catch (const std::exception& e) {
    if (m_debug == DebugMode::Vital | m_debug == DebugMode::Full)
      std::cerr << "Failed to check status: " << e.what() << std::endl;
    m_isAvailable = false;

    response = false;
  }

  RETURN_AND_RELEASE(m_linkMutex, response);
}

bool IPbusTarget::transceive(IPbusControlPacket& p, bool shouldResponseBeProcessed)
{
  pthread_mutex_lock(&m_linkMutex);

  if (m_isAvailable == false) {
    if (m_debug == DebugMode::Full) {
      std::cerr << "Device is not available" << std::endl;
    }
    RETURN_AND_RELEASE(m_linkMutex, false);
  }
  if (p.m_requestSize <= 1) {
    std::cerr << "Empty request" << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, true);
  }

  size_t send_bytes = 0;

  try {
    send_bytes = m_socket.send_to(boost::asio::buffer((char*)&p.m_request, p.m_requestSize * wordSize), m_remoteEndpoint);
  } catch (const std::exception& e) {
    std::cerr << "Sending packet failed: " << e.what() << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  if (send_bytes < p.m_requestSize * wordSize) {
    std::cerr << "Sending packer faild: " << send_bytes << " bytes was send instead of " << p.m_requestSize * wordSize << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  size_t bytes_recevied = receive((char*)&p.m_response, p.m_requestSize * wordSize);

  if (bytes_recevied == 64 && p.m_response[0] == m_status.header) {
    bytes_recevied = receive((char*)&p.m_response, p.m_requestSize * wordSize);
  }

  if (bytes_recevied == 0) {
    std::cerr << "Empty response from " << m_ipAddress << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else if (bytes_recevied / wordSize > p.m_responseSize || p.m_response[0] != p.m_request[0] || bytes_recevied % wordSize > 0) {
    std::cerr << "Incorrect response: received " << bytes_recevied << " bytes instead of " << p.m_responseSize * wordSize << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else {
    p.m_responseSize = uint16_t(bytes_recevied / wordSize); // response can be shorter then expected if a transaction wasn't successful
    if (m_debug == DebugMode::Full && shouldResponseBeProcessed) {
      std::cerr << "Processing response\n";
    } else if (m_debug == DebugMode::Full) {
      std::cerr << "Request will not be processed" << std::endl;
    }
    bool result = shouldResponseBeProcessed ? p.processResponse() : true;
    if (m_debug == DebugMode::Full && shouldResponseBeProcessed) {
      std::cerr << "Response processed: " << (result ? "success" : "failure") << std::endl;
    }
    p.reset();
    RETURN_AND_RELEASE(m_linkMutex, result);
  }
  RETURN_AND_RELEASE(m_linkMutex, false);
}

void IPbusTarget::intializeMutex(pthread_mutex_t& mutex)
{
  pthread_mutex_init(&mutex, NULL);
}

} // namespace ipbus