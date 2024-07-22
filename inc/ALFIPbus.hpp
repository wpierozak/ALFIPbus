#include"SWTelectronics.h"
#include<list>
class ALFIpbus
{
    public:
    ALFIpbus(std::string name);
    void init_link(std::string remote_address, int rport, int lport = 0);
    void main_loop();
    
    bool m_work;

    private:

    void start_server();

    std::string m_server_name;
    std::list<SWTelectronics> m_links;

    boost::asio::io_context m_io_context;
};