#ifndef IPBUSENUMS_H
#define IPBUSENUMS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <climits>

namespace ipbus
{

namespace enums
{

namespace packets
{
    enum PacketType { Control = 0,
                  Status = 1,
                  Resend = 2 };
}
namespace transactions
{    
    enum TransactionType { Read = 0,
                        Write = 1,
                        NonIncrementingRead = 2,
                        NonIncrementingWrite = 3,
                        RMWbits = 4,
                        RMWsum = 5,
                        ConfigurationRead = 6,
                        ConfigurationWrite = 7 };
}

}

}

#endif