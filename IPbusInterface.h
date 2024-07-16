
#include<mutex>
#include<boost/asio.hpp>
#include"IPbusControlPacket.h"

#define IO_BUFFER_SIZE 1024

class IPbusTarget
{
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

    // Async communication //

    void IPbusTarget::start_async_recv();
    void IPbusTarget::handle_recv(const boost::system::error_code& error, std::size_t bytes_transferred);

    char m_async_buffer[IO_BUFFER_SIZE];

public:

    IPbusTarget(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=0, uint16_t rport=50001);

    bool checkStatus();
    bool reopen();

    bool transcieve(IPbusControlPacket &p, bool shouldResponseBeProcessed = true);
    virtual void sync();
};

