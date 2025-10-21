## AlfIPbus
Paper: https://arxiv.org/abs/2510.11165

AlfIPbus was developed as a temporary Detector Control System (DCS) solution for the Fast Interaction Trigger (FIT) detector in the ALICE experiment. Its primary purpose is to serve as a software translator between the IPbus protocol, used in the previous DCS setup, and the new custom SWT FIT protocol in the updated system. AlfIPbus provides a fast and reliable translation mechanism and is fully compatible with FRED software.

## Building
### Dependencies
- BOOST 1.83.0
- DIM

### Instruction
```
git submodule update --recursive --init \
mkdir build                             \
cmake3 -S . -B build                    \
cmake3 --build build                    \
```

## Steps to Configure and Enable alfipbus service

1. **Copy the Service File**  
   First, copy the `alfipbus.service` file to the systemd unit file directory (the default is `/etc/systemd/system`). You can do this with the following command:
    ```  
    sudo cp alfipbus.service /etc/systemd/system/
    ```
2. **Edit the Service Configuration**
    Next, you need to set the necessary environment variables and other options in the service file. Use the command below to open the file for editing:
    ```
    sudo systemctl edit alfipbus.service
    ```

    In the editor, define the following environment variables:
    - NAME: Set the name of the service.
    - DEVICE_ADDRESS: Specify the device address.
    - LOG_FILE: Provide the path where the log file should be stored.
    - TIMEOUT: Define the timeout period for the service.
    - DIM_DNS_NODE: Set the DNS node details.
    
    Additionally, configure the following options:
    - User: Specify the user that should run the service by setting the User option.
    - WorkingDirectory: Set the directory where the log file will be created by setting the WorkingDirectory option.

    **AlfIPbus** executable file is expected to be located within /usr/local/bin. You may move it there or specify another **ExecStart** path.

3. **Enable the Service**
    Once the service configuration is complete, enable the service to start at boot with this command:
    ```
    sudo systemctl enable alfipbus.service
    ```
4. **Reload the Systemd Daemon**
    After editing and enabling the service, reload the systemd daemon to apply the changes:
    ```
    sudo systemctl daemon-reload
    ```

## Command line options

- `name/n [name]`- ALFIPbus server name
- `log_file/f [filename]` (`f`)- Log file name (if not specified, logs will be sent to std::cout)
- `link/l [IP address]:[remote port]` (`l`) - set the IP address and port for consecutive links (can be used multiple times)
- `t [time in miliseconds]` - IPbus request timeout (in miliseconds) 
- `v` - verbose

## SWT Frame

SwtLink is compatible with SWT frame designed for ALICE's FIT detector.

| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |

|Transaction type code | Transaction |
|----------------|--------------------|
|0x0 | read |
|0x1 | write |
|0x2 | RMWbits AND |
|0x3 | RMWbits OR |
|0x4 | RMW sum |
|0x8 | block read |
|0x9 | block read non-increment |

----
### READ

#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0000| addresss | ignored |
#### Response
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0000| addresss | data |

----
### WRITE 
#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0001| addresss | data |
#### Response
None

----
### RMW bits X <= (X & A) | B

#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0010| address | AND mask |
|0011|00000000|0011| address | OR mask |

#### Response
Only to first frame
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0010| address | data before operation |

----
### RMW Sum  X <= (X + A)

#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0100| address | data to sum |

#### Response
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|0100| address | data before operation |

----
#### Block read - incrementing

#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|1000| address | number of words |

#### Response
Each word is send in separate frame
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|1000| word address | data |

----
#### Block read - non-incrementing

#### Request
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|1001| address | number of words |

#### Response
Each word is send in separate frame
| SWT ID (4b) | NOT USED (8b) | Transaction type (4b) | ADDRESS (32b) | DATA (32b) |
|----|-----|-----|-----|----|
|0011|00000000|1001| address | data |

#### 

## Testing
Testing framework was provided by [frun36](https://github.com/frun36) and is available [here](https://github.com/frun36/alf-ipbus-tester).

### In-container-test
The test for developers is available within the `test/in-container-test` directory and can be executed using the `in-container-test.sh` script. Running this test requires the DIM DNS server to be running on your machine. The script builds the AlfIPbus within a container and subsequently builds a container for the testing framework.

The test is conducted using three containers: ALF, Mock, and Generator. The configuration for the test is stored in `test/in-container-test/common-storage/test-configuration.sh`. Logs from all three containers are output to the `test/in-container-test/common-storage/output` directory. If all tests pass, a message will be displayed at the end of the Generator log file.

