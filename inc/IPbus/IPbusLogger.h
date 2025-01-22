#include<iostream>

class IPbusLogger
{
public:
    static IPbusLogger& 

private:
    IPbusLogger(std::ostream& os = std::cout);

    std::ostream& m_os;
};