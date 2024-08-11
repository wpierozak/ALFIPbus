#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <iostream>
#include "IPbusMaster.h"
#include <boost/log/trivial.hpp>
#include <sys/socket.h>
#include <sstream>
namespace ipbus
{

IPbusMaster::IPbusMaster(boost::asio::io_context& ioContext, std::string address, uint16_t lport, uint16_t rport) : m_ioContext(ioContext),
                                                                                                                    m_localPort(lport),
                                                                                                                    m_remotePort(rport),
                                                                                                                    m_ipAddress(address),
                                                                                                                    m_localEndpoint(boost::asio::ip::udp::v4(), m_localPort),
                                                                                                                    m_remoteEndpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_ipAddress), m_remotePort)),
                                                                                                                    m_socket(ioContext),
                                                                                                                    m_timer(ioContext)

{
  openSocket();
  intializeMutex(m_linkMutex);
  intializeMutex(m_receiveStatusMutex);
  checkStatus();
}

bool IPbusMaster::openSocket()
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

bool IPbusMaster::reopen()
{
  if (m_socket.is_open()) {
    BOOST_LOG_TRIVIAL(debug) << "Socket is already open.";
    return true;
  }
  BOOST_LOG_TRIVIAL(debug) << "Socket is not open. Attempting to reopen...";
  return openSocket(); // Reopen the socket if it is not open
}

size_t IPbusMaster::receive(char* destBuffer, size_t maxSize)
{
  try {
    BOOST_LOG_TRIVIAL(debug) << "Synchronous receiving...";
   
    m_receivedSize = 0;
    m_error = boost::asio::error::would_block;
    m_receiveStatus = ReceiveStatus::Wait;

    m_socket.async_receive_from(boost::asio::buffer(destBuffer, maxSize), m_remoteEndpoint, boost::bind(&IPbusMaster::handleReceive, this, std::placeholders::_1, std::placeholders::_2));

    m_timer.expires_from_now(m_timeout);
    m_timer.async_wait(boost::bind(&IPbusMaster::handleDeadline, this));

    do {
      m_ioContext.run_one();
    } while (m_receiveStatus == ReceiveStatus::Wait);

    return m_receivedSize;

  } catch (const boost::system::system_error& e) {
    BOOST_LOG_TRIVIAL(error) << "Receive error: " << e.what();
    return 0;
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Error during receive operation: " << e.what();
    return 0;
  } catch (...) {
    BOOST_LOG_TRIVIAL(error) << "An unknown error occurred during receive operation.";
    return 0;
  }
  return 0;
}

void IPbusMaster::handleReceive(const boost::system::error_code& ec, std::size_t length)
{
  pthread_mutex_lock(&m_receiveStatusMutex);

  BOOST_LOG_TRIVIAL(debug) << "Message received: " << length << " bytes";

  m_error = ec;
  m_receivedSize = length;

  m_timer.cancel();
  m_receiveStatus = ReceiveStatus::Received;

  pthread_mutex_unlock(&m_receiveStatusMutex);
}

void IPbusMaster::handleDeadline()
{
  if (m_timer.expires_at() > boost::asio::deadline_timer::traits_type::now()) {
    m_timer.async_wait(boost::bind(&IPbusMaster::handleDeadline, this));
    return;
  }

  pthread_mutex_lock(&m_receiveStatusMutex);

  if (m_receiveStatus == ReceiveStatus::Wait) {
    BOOST_LOG_TRIVIAL(error) << "Request has timed out";

    m_receiveStatus = ReceiveStatus::Expired;
    //m_socket.cancel();
    m_timer.expires_at(boost::posix_time::pos_infin);
  }

  pthread_mutex_unlock(&m_receiveStatusMutex);
}

bool IPbusMaster::checkStatus()
{
  pthread_mutex_lock(&m_linkMutex);
  m_isAvailable = false;

  try {
    BOOST_LOG_TRIVIAL(info) << "Checking status of device at " << m_ipAddress << ":" << m_remotePort;

    m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remoteEndpoint);
    size_t bytes = receive((char*)&m_statusRespone, sizeof(m_statusRespone));

    if (m_error) {
      BOOST_LOG_TRIVIAL(error) << m_error.message();
    } else if (m_receiveStatus == ReceiveStatus::Expired) {
      BOOST_LOG_TRIVIAL(error) << "Status packet has not been received";
    } else if (bytes != sizeof(m_status)) {
      BOOST_LOG_TRIVIAL(error) << "Status packet is invalid - received " << bytes << " (expected " << sizeof(m_status) << ")";
    } else if (m_statusRespone.header != m_status.header) {
      BOOST_LOG_TRIVIAL(error) << "Received unexpected packet";
    } else {
      BOOST_LOG_TRIVIAL(info) << "Device at " << m_ipAddress << ":" << m_remotePort << " is available.";
      m_isAvailable = true;
    }

  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to check status of device at " << m_ipAddress << ":" << m_remotePort << ": " << e.what() << std::endl;
    m_isAvailable = false;
  }

  RETURN_AND_RELEASE(m_linkMutex, m_isAvailable);
}

bool IPbusMaster::transceive(IPbusRequest& request, IPbusResponse& response, bool shouldResponseBeProcessed)
{
  if (m_isAvailable == false) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: device at " << m_ipAddress << ":" << m_remotePort << " is not available";
  }

  pthread_mutex_lock(&m_linkMutex);

  if (request.getSize() <= 1) {
    BOOST_LOG_TRIVIAL(warning) << "Transceive: empty request";
    RETURN_AND_RELEASE(m_linkMutex, true);
  }

  size_t send_bytes = 0;

  try {
    send_bytes = m_socket.send_to(boost::asio::buffer((char*)request.getBuffer(), request.getSize() * wordSize), m_remoteEndpoint);
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: sending packet to " << m_ipAddress << ":" << m_remotePort << " failed: " << e.what();
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  if (send_bytes < request.getSize() * wordSize) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: Sending packet to " << m_ipAddress << ":" << m_remotePort << " failed: " << send_bytes << " bytes were sent instead of " << request.getSize() * wordSize;
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  response.reset();
  size_t bytes_recevied = receive((char*)response.getBuffer(), maxPacket * wordSize);

  if (m_receiveStatus == ReceiveStatus::Expired) {
    RETURN_AND_RELEASE(m_linkMutex, false);
  }

  if (bytes_recevied == 64 && response[0] == m_status.header) {
    bytes_recevied = receive((char*)response.getBuffer(), maxPacket * wordSize);
  }

  if (bytes_recevied == 0) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: Empty response from " << m_ipAddress << ":" << m_remotePort;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else if (bytes_recevied / wordSize > request.getExpectedResponseSize() || bytes_recevied % wordSize > 0) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: Incorrect response from " << m_ipAddress << ":" << m_remotePort << ": received " << bytes_recevied << " bytes instead of " << request.getSize() * wordSize;
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else if (response[0] != request[0]) {
    BOOST_LOG_TRIVIAL(error) << "Transceive: Invalid incoming packet header";
    RETURN_AND_RELEASE(m_linkMutex, false);
  } else {
    response.setSize(bytes_recevied / wordSize);

    if (shouldResponseBeProcessed) {
      BOOST_LOG_TRIVIAL(debug) << "Transceive: Processing response";
    } else {
      BOOST_LOG_TRIVIAL(debug) << "Transceive: Response will not be processed";
    }

    bool result = shouldResponseBeProcessed ? processResponse(request, response) : true;
    if (!result) {
      BOOST_LOG_TRIVIAL(error) << "Transceive: Failed to process response";

      std::string reqString = "Request:\n";
      for (int i = 0; i < request.getSize(); i++) {
        reqString += boost::str(boost::format("%1$08x\n") % request[i]);
      }

      BOOST_LOG_TRIVIAL(debug) << reqString;

      std::string resString = "Response:\n";
      for (int i = 0; i < response.getSize(); i++) {
        resString += boost::str(boost::format("%1$08x\n") % response[i]);
      }

      BOOST_LOG_TRIVIAL(debug) << resString;
    }

    RETURN_AND_RELEASE(m_linkMutex, result);
  }
  RETURN_AND_RELEASE(m_linkMutex, false);
}

bool IPbusMaster::processResponse(IPbusRequest& request, IPbusResponse& response)
{
  uint16_t idxRequest = 1;
  uint16_t idxResponse = 1;

  for (uint16_t idx = 0; idx < request.getTransactionNumber(); idx++) {
    TransactionHeader* headerRequest = (TransactionHeader*)request.getBuffer() + (idxRequest++);
    TransactionHeader* headerResponse = (TransactionHeader*)response.getBuffer() + (idxResponse++);

    if (headerResponse->protocolVersion != 2 || headerResponse->transactionID != idx ||
        headerResponse->typeID != headerRequest->typeID) {

      BOOST_LOG_TRIVIAL(error) << boost::str(
        boost::format(
          "Unexpected header for transaction %1$d: got %2$08x, expected %3$08x") %
        idx % *reinterpret_cast<uint32_t*>(headerResponse) % *reinterpret_cast<uint32_t*>(headerRequest));
      
      return false;
    }

    if (headerResponse->words > 0) {
      switch (headerResponse->typeID) {
        case enums::transactions::Read:
        case enums::transactions::NonIncrementingRead:
        case enums::transactions::ConfigurationRead: {
          if (headerResponse->words != headerRequest->words) {
            BOOST_LOG_TRIVIAL(error) << "Read transaction failed: expected " << headerRequest->words << "words, received: " << headerResponse->words;
            return false;
          } else {
            if (request.getDataOut(idx) != nullptr) {
              memcpy(request.getDataOut(idx), headerResponse + 1, headerResponse->words * wordSize);
            }
            idxResponse += headerResponse->words;
            idxRequest += 1;
          }
        } break;

        case enums::transactions::RMWbits: {
          if (headerResponse->words != 1) {
            BOOST_LOG_TRIVIAL(error) << "RMWbits transaction no " << idx << " failed";
            return false;
          }
          if (request.getDataOut(idx) != nullptr) {
            memcpy(request.getDataOut(idx), response.getBuffer() + idxResponse, wordSize);
            idxResponse++;
            idxRequest += 3;
          }
        } break;
        case enums::transactions::RMWsum: {
          if (headerResponse->words != 1) {
            BOOST_LOG_TRIVIAL(error) << "RMWsum transaction no " << idx << " failed";
            return false;
          }
          if (request.getDataOut(idx) != nullptr) {
            memcpy(request.getDataOut(idx), response.getBuffer() + idxResponse, wordSize);
            idxResponse++;
            idxRequest += 2;
          }
        } break;

        case enums::transactions::Write:
        case enums::transactions::NonIncrementingWrite:
        case enums::transactions::ConfigurationWrite:
          idxRequest += headerRequest->words + 1;
          break;

        default:
          BOOST_LOG_TRIVIAL(error) << "Unknown transaction type: response cannot be processed";
          return false;
      }
    }

    if (headerResponse->infoCode != 0) {
      std::string message = headerResponse->infoCodeString();
      BOOST_LOG_TRIVIAL(error) << "Transaction response error: " << message;
      return false;
    }
  }

  return true;
}

void IPbusMaster::intializeMutex(pthread_mutex_t& mutex)
{
  pthread_mutex_init(&mutex, NULL);
}

void IPbusMaster::setTimeout(boost::posix_time::milliseconds timeout)
{
  m_timeout = timeout;
}

boost::posix_time::milliseconds IPbusMaster::getTimeout() const
{
  return m_timeout;
}

} // namespace ipbus
