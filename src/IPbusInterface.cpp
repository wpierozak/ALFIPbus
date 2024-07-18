#include"IPbusInterface.h"
#include <boost/bind.hpp>
#include<iostream>

void* IPbusTarget::io_thread_function(void* object)
{
    IPbusTarget* interface = (IPbusTarget*) object;
    
    pthread_mutex_lock(&interface->m_thread_state_mutex);
    interface->m_is_running = true;
    pthread_mutex_unlock(&interface->m_thread_state_mutex);

    if(interface->debug_mode() == DebugMode::Full) std::cerr << "IO thread function - running" << std::endl;
    interface->io_context_run();

    pthread_mutex_lock(&interface->m_thread_state_mutex);
    interface->m_is_running = false;
    pthread_mutex_unlock(&interface->m_thread_state_mutex);

    return NULL;
}


IPbusTarget::IPbusTarget(boost::asio::io_context & io_context, std::string address, uint16_t lport, uint16_t rport):
    m_io_context(io_context),
    m_localport(lport), 
    m_remoteport(rport),
    m_IPaddress(address),
    m_local_endpoint(boost::asio::ip::udp::v4(), m_localport),
    m_remote_endpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(m_IPaddress), m_remoteport)),
    m_socket(io_context),

    m_timer(m_io_context)
{
    open_socket();
    start_timer();

    intialize_mutex(m_link_mutex);
    intialize_mutex(m_timer_mutex);
    intialize_mutex(m_thread_state_mutex);

    checkStatus();

    start_io_thread();
}

IPbusTarget::~IPbusTarget()
{
    shutdown_io();
}

bool IPbusTarget::open_socket()
{
    if(m_debug == DebugMode::Vital || m_debug == DebugMode::Full) std::cerr << "Attempting to open socket..."<< std::endl;
    m_socket.open(boost::asio::ip::udp::v4());

    if(m_socket.is_open())
    {
        if(m_debug == DebugMode::Vital || m_debug == DebugMode::Full) std::cerr << "Socket successfully opened" << std::endl;
        return true;
    }
    else
    {
        if(m_debug == DebugMode::Vital || m_debug == DebugMode::Full) std::cerr << "Failed to open socket."  << std::endl;
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
    return open_socket();  // Reopen the socket if it is not open
}


size_t IPbusTarget::sync_recv(char* dest_buffer, size_t max_size) {
    try {
        if(m_debug == DebugMode::Full) std::cerr << "Synchronized receiving..." << std::endl;
        size_t bytes_transferred = m_socket.receive_from(boost::asio::buffer(dest_buffer, IO_BUFFER_SIZE), m_remote_endpoint);
        if(m_debug == DebugMode::Full) std::cerr << "Message received: " << bytes_transferred << " bytes" << std::endl;
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
   pthread_mutex_lock(&m_link_mutex);
   bool response = true;

    try {
        if(m_debug == DebugMode::Vital | m_debug == DebugMode::Full) std::cerr << "Checking status of device at " << m_IPaddress << std::endl;
        // Send a status packet to the remote endpoint
        m_socket.send_to(boost::asio::buffer(&m_status, sizeof(m_status)), m_remote_endpoint);
        sync_recv((char*) &m_status_respone, sizeof(m_status_respone));
        if(m_debug == DebugMode::Vital || m_debug == DebugMode::Full) std::cerr << "Status check successful: Device is available." << std::endl;
        is_available = true;

        response = true;

    } catch (const std::exception& e) {
        if(m_debug == DebugMode::Vital | m_debug == DebugMode::Full) std::cerr << "Failed to check status: " << e.what() << std::endl;
        is_available = false;

        response = false;
    }

    RETURN_AND_RELEASE(m_link_mutex, response);
}

bool IPbusTarget::transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed)
{
    pthread_mutex_lock(&m_link_mutex);

    if(is_available == false)
    {
        if(m_debug == DebugMode::Full)
        {
            std::cerr << "Device is not available" << std::endl;
        }
        RETURN_AND_RELEASE(m_link_mutex, false);
    }
    if(p.requestSize <= 1)
    {
        std::cerr << "Empty request" << std::endl;
        RETURN_AND_RELEASE(m_link_mutex, true);
    }

    size_t send_bytes =  0;

    try
    {
        send_bytes = m_socket.send_to(boost::asio::buffer((char*)&p.request, p.requestSize * wordSize), m_remote_endpoint);
    }
    catch(const std::exception& e)
    {
        std::cerr<< "Sending packet failed: " << e.what() << std::endl;
        RETURN_AND_RELEASE(m_link_mutex, false);
    }

    if(send_bytes < p.requestSize*wordSize)
    {
        std::cerr << "Sending packer faild: " << send_bytes << " bytes was send instead of " << p.requestSize*wordSize << std::endl;
        RETURN_AND_RELEASE(m_link_mutex, false);
    }

    size_t bytes_recevied  = sync_recv((char*)&p.response, p.requestSize*wordSize);

    if(bytes_recevied == 64 && p.response[0] == m_status.header)
    {
        bytes_recevied = sync_recv((char*)&p.response, p.requestSize*wordSize);
    }

    if(bytes_recevied == 0)
    {
        std::cerr << "Empty response from " << m_IPaddress << std::endl;
        RETURN_AND_RELEASE(m_link_mutex, false);
    }
    else if (bytes_recevied / wordSize > p.responseSize || p.response[0] != p.request[0] || bytes_recevied % wordSize > 0) {
            std::cerr << "Incorrect response: received " << bytes_recevied << " bytes instead of " << p.responseSize*wordSize << std::endl;
            RETURN_AND_RELEASE(m_link_mutex, false);
    } else {
            p.responseSize = uint16_t(bytes_recevied / wordSize); //response can be shorter then expected if a transaction wasn't successful
            if(m_debug == DebugMode::Full && shouldResponseBeProcessed) 
            {
                std::cerr << "Processing response\n";
            }
            else if(m_debug == DebugMode::Full)
            {
                std::cerr << "Request will not be processed" << std::endl;
            } 
            bool result = shouldResponseBeProcessed ? p.processResponse() : true;
            if(m_debug == DebugMode::Full && shouldResponseBeProcessed)
            {
                std::cerr << "Response processed: " << (result ? "success": "failure") << std::endl;
            }
            p.reset();
            RETURN_AND_RELEASE(m_link_mutex, result);
    }
    RETURN_AND_RELEASE(m_link_mutex, false);
}

void IPbusTarget::reset_timer()
{
    pthread_mutex_lock(&m_timer_mutex);
    if(m_stop_timer)
    {   
        m_timer.cancel();
        RETURN_AND_RELEASE(m_timer_mutex, )
    }
    m_timer.expires_from_now(m_tick);
    m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));

    RETURN_AND_RELEASE(m_timer_mutex, )
}

void IPbusTarget::sync(const boost::system::error_code& error)
{
    checkStatus();
    reset_timer();
}

void IPbusTarget::io_context_run()
{
    m_io_context.run();
}

void IPbusTarget::intialize_mutex(pthread_mutex_t& mutex)
{
    pthread_mutex_init(&mutex, NULL);
}

void IPbusTarget::start_io_thread()
{
    pthread_create(&m_thread, NULL, IPbusTarget::io_thread_function, (void*) this);
}

void IPbusTarget::shutdown_io()
{
    stop_timer();
    m_socket.close();
    m_io_context.stop();
    pthread_join(m_thread, NULL);
}

void IPbusTarget::start_timer()
{
    pthread_mutex_lock(&m_timer_mutex);

    m_stop_timer = false;
    m_timer.expires_from_now(m_tick);
    m_timer.async_wait(boost::bind(&IPbusTarget::sync, this, boost::asio::placeholders::error));

    pthread_mutex_unlock(&m_timer_mutex);
}

void IPbusTarget::stop_timer()
{
    pthread_mutex_lock(&m_timer_mutex);
    m_stop_timer = true;
    pthread_mutex_unlock(&m_timer_mutex);
}