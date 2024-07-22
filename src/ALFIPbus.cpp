#include"ALFIPbus.hpp"

ALFIPbus::ALFIPbus(std::string name):
    m_server_name(name), m_work(true)
{

}

void ALFIPbus::init_link(std::string remote_address, int rport, int lport)
{
    std::string serial = m_server_name + "/SERIAL_0/LINK_";
    std::string swt_seq = "/SWT_SEQUENCE";
    int number = m_links.size();
    m_links.emplace_back(serial + std::to_string(number) + swt_seq, m_io_context, remote_address, rport, lport);
}

void ALFIPbus::start_server()
{
    DimServer::start(m_server_name.c_str());
    main_loop();
}

void ALFIPbus::main_loop()
{
    while(m_work)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1000));
    }
}