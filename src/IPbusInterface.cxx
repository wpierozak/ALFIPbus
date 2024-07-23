#include <boost/bind.hpp>
#include <iostream>
#include "IPbusInterface.h"

namespace ipbus
{

void* IPbusTarget::ioThreadFunction(void* object)
{
  IPbusTarget* interface = (IPbusTarget*)object;

  pthread_mutex_lock(&interface->m_threadStateMutex);
  interface->m_isRunning = true;
  pthread_mutex_unlock(&interface->m_threadStateMutex);

  if (interface->debugMode() == DebugMode::Full)
    std::cerr << "IO thread function - running" << std::endl;
  interface->ioContextRun();

  pthread_mutex_lock(&interface->m_threadStateMutex);
  interface->m_isRunning = false;
  pthread_mutex_unlock(&interface->m_threadStateMutex);

  return NULL;
}

IPbusTarget::IPbusTarget(boost::asio::io_context& io_context, std::string address, uint16_t lport, uint16_t rport) : m_ioContext(io_context),
                                                                                                                     m_localPort(lport),
                                                                                                                     m_remotePort(rport),
                                                                                                                     m_ipAddress(address),
                                                                                                                     m_localEndpoint(boost::asio::ip::udp::v4(), m_localPort),
                                                                                                                     m_remoteEndpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_ipAddress), m_remotePort)),
                                                                                                                     m_socket(io_context),

                                                                                                                     m_timer(m_ioContext)
{
  openSocket();

  intializeMutex(m_linkMutex);
  intializeMutex(m_timerMutex);
  intializeMutex(m_threadStateMutex);

  checkStatus();
}

IPbusTarget::~IPbusTarget()
{
  shutdownIo();
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

size_t IPbusTarget::sync_recv(char* dest_buffer, size_t max_size)
{
  try {
    if (m_debug == DebugMode::Full)
      std::cerr << "Synchronized receiving..." << std::endl;
    size_t bytes_transferred = m_socket.receive_from(boost::asio::buffer(dest_buffer, IO_BUFFER_SIZE), m_remoteEndpoint);
    if (m_debug == DebugMode::Full)
      std::cerr << "Message received: " << bytes_transferred << " bytes" << std::endl;
    return bytes_transferred;
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
    sync_recv((char*)&m_statusRespone, sizeof(m_statusRespone));
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

bool IPbusTarget::transcieve(IPbusControlPacket& p, bool shouldResponseBeProcessed)
{
  pthread_mutex_lock(&m_linkMutex);

  if (m_isAvailable == false) {
    if (m_debug == DebugMode::Full) {
      std::cerr << "Device is not available" << std::endl;
    }
    RETURN_AND_RELEASE(m_linkMutex, false);
  }
  if (p.requestSize <= 1) {
    std::cerr << "Empty request" << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, true);
  }

  size_t send_bytes = 0;

  try {
    send_bytes = m_socket.send_to(boost::asio::buffer((char*)&p.request, p.requestSize * wordSize), m_remoteEndpoint);
  } catch (const std::exception& e) {
    std::cerr << "Sending packet failed: " << e.what() << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  if (send_bytes < p.requestSize * wordSize) {
    std::cerr << "Sending packer faild: " << send_bytes << " bytes was send instead of " << p.requestSize * wordSize << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  size_t bytes_recevied = sync_recv((char*)&p.response, p.requestSize * wordSize);

  if (bytes_recevied == 64 && p.response[0] == m_status.header) {
    bytes_recevied = sync_recv((char*)&p.response, p.requestSize * wordSize);
  }

  if (bytes_recevied == 0) {
    std::cerr << "Empty response from " << m_ipAddress << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else if (bytes_recevied / wordSize > p.responseSize || p.response[0] != p.request[0] || bytes_recevied % wordSize > 0) {
    std::cerr << "Incorrect response: received " << bytes_recevied << " bytes instead of " << p.responseSize * wordSize << std::endl;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else {
    p.responseSize = uint16_t(bytes_recevied / wordSize); // response can be shorter then expected if a transaction wasn't successful
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

void IPbusTarget::resetTimer()
{
  pthread_mutex_lock(&m_timerMutex);
  if (m_stopTimer) {
    m_timer.cancel();
    RETURN_AND_RELEASE(m_timerMutex, )
  }
  m_timer.expires_from_now(m_tick);
  m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));

  RETURN_AND_RELEASE(m_timerMutex, )
}

void IPbusTarget::sync(const boost::system::error_code& error)
{
  checkStatus();
  resetTimer();
}

void IPbusTarget::ioContextRun()
{
  m_ioContext.run();
}

void IPbusTarget::intializeMutex(pthread_mutex_t& mutex)
{
  pthread_mutex_init(&mutex, NULL);
}

void IPbusTarget::startIoThread()
{
  pthread_create(&m_thread, NULL, IPbusTarget::ioThreadFunction, (void*)this);
}

void IPbusTarget::shutdownIo()
{
  stopTimer();
  m_socket.close();
  if (m_isRunning == true) {
    m_ioContext.stop();
    pthread_join(m_thread, NULL);
  }
}

void IPbusTarget::startTimer()
{
  pthread_mutex_lock(&m_timerMutex);
  startIoThread();
  m_stopTimer = false;
  m_timer.expires_from_now(m_tick);
  m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));

  pthread_mutex_unlock(&m_timerMutex);
}

void IPbusTarget::stopTimer()
{
  pthread_mutex_lock(&m_timerMutex);
  m_stopTimer = true;
  pthread_mutex_unlock(&m_timerMutex);
}

} // namespace ipbus