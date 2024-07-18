
#include<mutex>
#include<boost/asio.hpp>
#include<memory>
#include"IPbusControlPacket.h"

#define IO_BUFFER_SIZE 1024

class IPbusTarget
{
    public:

    enum class DebugMode{Full = 0, Vital = 1, Non};

    private:

    boost::asio::io_context& m_io_context;

    uint16_t m_localport;
    uint16_t m_remoteport;
    std::mutex m_transcieve_mutex;

    std::string m_IPaddress;
    bool isOnline = false;

    boost::asio::ip::udp::endpoint m_local_endpoint;
    boost::asio::ip::udp::endpoint m_remote_endpoint;

    boost::asio::ip::udp::socket m_socket;

    const StatusPacket m_status;
    StatusPacket m_status_respone;
    IPbusControlPacket m_packet;

    bool is_available;

    bool open_socket();

    // Sync communication

    size_t sync_recv(char* dest_buffer, size_t max_size);

    char m_buffer[IO_BUFFER_SIZE];

    // Periodic communication //

    void io_context_run();
    std::shared_ptr<std::thread> m_thread{nullptr};

    void start_io_thread();
    void stop_io_thread();


    bool m_stop_timer{false};
    boost::asio::steady_timer m_timer;
    std::chrono::seconds m_tick{1};

    std::mutex m_timer_mutex;
    void reset_timer();

    virtual void sync(const boost::system::error_code& error);

    char m_async_buffer[IO_BUFFER_SIZE];

    DebugMode m_debug{DebugMode::Vital};

public:

    //enum class DebugMode{Full = 0, Vital = 1, Non};

    IPbusTarget(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=0, uint16_t rport=50001);
    ~IPbusTarget();
    void start_timer();
    void stop_timer();

    bool checkStatus();
    bool reopen();

    bool transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed = true);

    std::chrono::seconds timer_tick() const {return m_tick;}
    void timer_tick(std::chrono::seconds tick) {m_tick = tick;}

    DebugMode debug_mode() const {return m_debug;}
    void debug_mode(DebugMode mode) {m_debug = mode;}

    bool isIPbusOK() { return is_available;}
};

