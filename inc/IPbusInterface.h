#ifndef IPbusInterface_H
#define IPbusInterface_H

#include<mutex>
#include<boost/asio.hpp>
#include<memory>
#include<pthread.h>

#include"IPbusControlPacket.h"

/* Sync and async buffer size */
#define IO_BUFFER_SIZE 1024

/* Should be used instead of simple "return" in every method that locks m_link_mutex */
#define RETURN_AND_RELEASE(mutex, statement) pthread_mutex_unlock(&mutex); return statement;

namespace IPbus
{

class IPbusTarget
{
    public:

    enum class DebugMode{Full = 0, Vital = 1, Non};

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
    bool m_isAvailable{false};

    // IPBUS transaction //

    protected:
    IPbusControlPacket m_packet;

    private:

    bool openSocket();

    // Sync communication

    size_t sync_recv(char* dest_buffer, size_t max_size);
   
    // Periodic communication //

    void ioContextRun();

    pthread_t m_thread;
    bool m_isRunning{false};
    void startIoThread();
    void shutdownIo();

    pthread_mutex_t m_linkMutex;
    pthread_mutex_t m_timerMutex;
    pthread_mutex_t m_threadStateMutex;
    void intializeMutex(pthread_mutex_t& mutex);

    bool m_stopTimer{false};
    boost::asio::steady_timer m_timer;
    std::chrono::seconds m_tick{1};

    void stopTimer();
    void resetTimer();

    virtual void sync(const boost::system::error_code& error);

    char m_asyncBuffer[IO_BUFFER_SIZE];

    DebugMode m_debug{DebugMode::Vital};

public:

    //enum class DebugMode{Full = 0, Vital = 1, Non};

    IPbusTarget(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=0, uint16_t rport=50001);
    ~IPbusTarget();

    bool checkStatus();
    bool reopen();

    void startTimer();

    bool transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed = true);

    std::chrono::seconds timerTick() const {return m_tick;}
    void timerTick(std::chrono::seconds tick) {m_tick = tick;}

    DebugMode debugMode() const {return m_debug;}
    void debugMode(DebugMode mode) {m_debug = mode;}

    bool isIPbusOK() { return m_isAvailable;}

    static void* ioThreadFunction(void* object);
};

}

#endif