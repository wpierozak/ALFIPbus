
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

    boost::asio::ip::udp::endpoint m_remote_endpoint;

    boost::asio::ip::udp::socket m_socket;

    const StatusPacket m_status;
    IPbusControlPacket m_packet;

    bool open_socket();
    void start_async_recv();
    void handle_recv(const boost::system::error_code& error, std::size_t bytes_transferred);

    char m_buffer[IO_BUFFER_SIZE];
public:

    IPbusTarget(boost::asio::io_context & io_context, std::string address = "172.20.75.175", uint16_t lport=17, uint16_t rport=50001);

    bool checkStatus();
    bool reopen();

};

