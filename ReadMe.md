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

## SWT Frame

SWTelectronics is compatible with SWT frame designed for FIT ALICE detector.

```
    0111    0000000001  11001100110011001100110011001100 00110011001100110011001100110011
   SWT ID     CONTROL               ADDRESS                         DATA

```

## Message format

Communication between ALF and FRED is governed by a few rules. However, it is essential to understand the message sent from FRED to ALF regarding the FRED sequence file.. Let's analyze an example of reading one register. The FRED configuration file may be written as follows:
```
00000001004100400000000@OUT
```
@OUT instructs FRED to save the response to the preceding message in the output variable OUT. If only one output variable is defined, its value will be published as the service value. WARNING: FRED uses only the lower 16 bits. If you want to publish 32-bit data, you should use MAPI message handling.

FRED translate sequence to request.
```
reset
00000001004100400000000,write
read
```

`reset`,`write` and `read` are **CRU-related instructions**
- `reset` is sent as the initial operation of every request 
- `<DATA>,write` writes DATA to SWT registers on the CRU; FRED includes it in the request for each line of the sequence.
- `read` reads the content of SWT registers; FRED includes it in the request for each line of the sequence followed by the output variable reference.

FRED expects **reception of the same number of lines (excluding `reset` line)** formatted with regard to strict rules.
- Successfull execution of whole sequence must be indicated by `success`
- Failure execution must be indicated by `failure`
- For each `write` instruction, `0` line must resend
- For each `read` instruction, 76 bit hexadecimal value must be resend

Then the response to the request from example should be:

```
success 0
000000010041004BADCAFEE"
```

Another example.
- Sequence file.
```
000000110041004BADCAFEE
00000001004100400000000@OUT
```
- Request
```
reset
000000110041004BADCAFEE,write
00000001004100400000000,write
read

```
- Response
```
success 0
0
000000010041004BADCAFEE
```
