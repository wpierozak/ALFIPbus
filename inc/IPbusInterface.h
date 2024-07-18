
#include<mutex>
#include<boost/asio.hpp>
#include<memory>
#include<pthread.h>

#include"IPbusControlPacket.h"

/* Sync and async buffer size */
#define IO_BUFFER_SIZE 1024

/* Should be used instead of simple "return" in every method that locks m_link_mutex */
#define RETURN_AND_RELEASE(mutex, statement) pthread_mutex_unlock(&mutex); return statement;

class IPbusTarget
{
    public:

    enum class DebugMode{Full = 0, Vital = 1, Non};

    private:

    // BOOST ASIO //

    boost::asio::io_context& m_io_context;

    uint16_t m_localport;
    uint16_t m_remoteport;

    std::string m_IPaddress;

    boost::asio::ip::udp::endpoint m_local_endpoint;
    boost::asio::ip::udp::endpoint m_remote_endpoint;

    boost::asio::ip::udp::socket m_socket;

    // IPBUS status //

    const StatusPacket m_status;
    StatusPacket m_status_respone;
    bool is_available{false};

    // IPBUS transaction //

    IPbusControlPacket m_packet;
    bool open_socket();

    // Sync communication

    size_t sync_recv(char* dest_buffer, size_t max_size);
   
    // Periodic communication //

    void io_context_run();

    pthread_t m_thread;
    bool m_is_running{false};
    void start_io_thread();
    void shutdown_io();

    pthread_mutex_t m_link_mutex;
    pthread_mutex_t m_timer_mutex;
    pthread_mutex_t m_thread_state_mutex;
    void intialize_mutex(pthread_mutex_t& mutex);

    bool m_stop_timer{false};
    boost::asio::steady_timer m_timer;
    std::chrono::seconds m_tick{1};

    void start_timer();
    void stop_timer();
    void reset_timer();

    virtual void sync(const boost::system::error_code& error);

    char m_async_buffer[IO_BUFFER_SIZE];

    DebugMode m_debug{DebugMode::Vital};

public:

    //enum class DebugMode{Full = 0, Vital = 1, Non};

    IPbusTarget(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=0, uint16_t rport=50001);
    ~IPbusTarget();

    bool checkStatus();
    bool reopen();

    bool transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed = true);

    std::chrono::seconds timer_tick() const {return m_tick;}
    void timer_tick(std::chrono::seconds tick) {m_tick = tick;}

    DebugMode debug_mode() const {return m_debug;}
    void debug_mode(DebugMode mode) {m_debug = mode;}

    bool isIPbusOK() { return is_available;}

    static void* io_thread_function(void* object);
};

