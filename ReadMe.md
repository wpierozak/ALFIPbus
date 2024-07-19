## Description

The IpbusSWT repository provides an implementation of SWT-to-IPbus translation dedicated for use with FRED Alice software. The core of the solution is the **SWTelectronics** class, which inherits from the **IPbusTarget** (IPbus submodule) and the **DimRpcParallel** (DimRpcParallel submodule). SWTelectronics is designed to provide a single service in a ALF-like manner and to receive messages in a format provided by FRED to ALF. 

### Example
Example code is provided in the **flp_test_server.cpp** file, content of the file is also included below.

```
#include"SWTelectronics.h"
#include<dim/dis.hxx>
#include<chrono>
#include<thread>
#include<ctime>
int main(int argc, const char** argv)
{
    
    boost::asio::io_context io_context;

    SWTelectronics target(argv[2], io_context);
    target.debug_mode(IPbusTarget::DebugMode::Full, "172.20.75.175", 50001);

    DimServer::start(argv[1]);
    for(int i = 0; i < std::stoi(argv[3]); i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
```

To use the SWTelectronics class, follow these basic steps:

1. Create a boost::asio::io_context variable (it must exist for the entire program's lifetime).
2. Create a SWTelectronics object by providing the RPC name, io_context variable, remote device address, and port it listens to.
3. (Optional) Set the level of detail for the debug information.
4. Start the DimServer.
5. Create a loop.


## Building

### Requirements
- C++20
- boost::asio
- pthreads
- DIM (https://dim.web.cern.ch/index.html)

### Step-by-step

If you already pulled repository and all submodules, the you need to:

1. Build DimRpcParallel library
```
cmake3 DimRpcParallel
cmake3 --build DimRpcParallel
```
2. Build IPbus library
```
cmake3 IPbus
cmake3 --build IPbus
```
3. Build IpbusSWT
```
cmake3 .
cmake3 --build .
```

## Message format

### Input message format (from FRED)
```
reset\n<SWT_PAYLOAD>,write\nread
```

### Output message format (to FRED)

#### Success
```
success 0\n0x00000000000<PAYLOAD>
```
#### Failute
```
failure
```