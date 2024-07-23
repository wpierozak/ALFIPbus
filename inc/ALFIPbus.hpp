#include"SwtLink.h"
#include<list>
class ALFIPbus
{
    public:
    ALFIPbus(std::string name);
    void init_link(std::string remote_address, int rport, int lport = 0);
   

    void start_server();
    
    bool m_work;

    private:

    void main_loop();
    std::string m_server_name;
    std::list<fit_swt::SwtLink> m_links;

    boost::asio::io_context m_io_context;
};