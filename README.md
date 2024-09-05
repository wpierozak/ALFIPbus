## Description

The IpbusSWT repository provides an implementation of SWT-to-IPbus translation dedicated for use with FRED Alice software. The core of the solution is the **SwtLink** class, which inherits from the **IPbusTarget** (IPbus submodule) and the **DimRpcParallel** (DimRpcParallel submodule). SwtLink is designed to provide a single service in a ALF-like manner and to receive messages in a format provided by FRED to ALF. 

IpbusSWT project is an integral part of the AlfIPbus project. 

## Building

### Requirements
- C++20
- boost::asio
- DIM (https://dim.web.cern.ch/index.html)

### Instruction

```
git submodule update --recursive --remote   \
mkdir build                                 \
cmake3 -S . -B build                        \
cmake3 --build build                        
```

## Example
Example code is provided in the **flp_test_server.cpp** file, content of the file is also included below.

```
#include"SwtLink.h"
#include<dim/dis.hxx>
#include<chrono>
#include<thread>
#include<ctime>
int main(int argc, const char** argv)
{
    
    boost::asio::io_context io_context;

    fit_swt::SwtLink target(argv[2], io_context, "127.0.0.1", 50000);

    DimServer::start(argv[1]);
    for(int i = 0; i < std::stoi(argv[3]); i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
```

To use the SwtLink class, follow these basic steps:

1. Create a boost::asio::io_context variable (it must exist for the entire program's lifetime).
2. Create a SwtLink object by providing the RPC name, io_context variable, remote device address, and port it listens to.
3. Start the DimServer.
4. Create a loop.


## SWT Frame

SwtLink is compatible with SWT frame designed for ALICE's FIT detector.

```
Total: 80b
| SWT ID (4b) | NOT USED (9b) | MASKED (2b) | READ/WRITE (1b) |             ADDRESS (32b)                 |                 DATA (32b)              |
    0011         0000 0000 0        00              1           1100 1100 1100 1100 1100 1100 1100 1100      0011 0011 0011 0011 0011 0011 0011 0011
```

### IPbus operations in SWT
```
MM | R/W | ADDRESS | DATA |  ->  RESPONSE
2b | 1b  | 32b     | 32b  |  ->  ... + 32b
```

#### READ (non-incrementing) (FIFO)
```
MM | R/W | ADDRESS | DATA     |  ->  RESPONSE
---------------------------------------------
00 | 0   | ADDRESS | DONTCARE |  ->  DATA
00 | 0   | ADDRESS | DONTCARE |  ->  DATA
```

#### READ (incrementing)
```
MM | R/W | ADDRESS   | DATA     |  ->  RESPONSE
-----------------------------------------------
00 | 0   | ADDRESS   | DONTCARE |  ->  DATA
00 | 0   | ADDRESS+1 | DONTCARE |  ->  DATA
```

#### WRITE (non-incrementing) (FIFO)
```
MM | R/W | ADDRESS | DATA |  ->  RESPONSE
-----------------------------------------
00 | 1   | ADDRESS | DATA |  ->  OK
00 | 1   | ADDRESS | DATA |  ->  OK
```

#### WRITE (incrementing)
```
MM | R/W | ADDRESS   | DATA |  ->  RESPONSE
-------------------------------------------
00 | 1   | ADDRESS   | DATA |  ->  OK
00 | 1   | ADDRESS+1 | DATA |  ->  OK
```

#### RMW bits X <= (X & A) | B
```
MM | R/W | ADDRESS   | DATA     |  ->  RESPONSE
-----------------------------------------------
01 | 0   | ADDRESS   | AND_MASK |  ->  DATA_PRE     # READ_AND
01 | 1   | ADDRESS   | OR_MASK  |  ->  OK           # WRITE_OR
```


#### RMW Sum  X <= (X + A)
```
MM | R/W | ADDRESS   | DATA     |  ->  RESPONSE
-----------------------------------------------
10 | 0   | ADDRESS   | SUM_TERM |  ->  DATA_PRE     # READ_SUM
```

## Message format

Communication between ALF and FRED is governed by a few rules. However, it is essential to understand the message sent from FRED to ALF regarding the FRED sequence file.. Let's analyze an example of reading one register. The FRED configuration file may be written as follows:
```
00000001004100400000000@OUT
```
`@OUT` instructs FRED to save the response to the preceding message in the output variable `OUT`. If only one output variable is defined, its value will be published as the service value. **WARNING**: FRED uses only the lower 16 bits. If you want to publish 32-bit data, you should use MAPI message handling.

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

FRED expects **reception of the same number of lines** formatted with regard to strict rules.
- Successfull execution of whole sequence must be indicated by `success`
- Failure execution must be indicated by `failure`
- For each `write` instruction, `0` line must resend
- For each `read` instruction, 76 bit hexadecimal value must be resend

Then the response to the request from example should be:

```
success 
0
000000010041004BADCAFEE
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
success 
0
0
000000010041004BADCAFEE
```


