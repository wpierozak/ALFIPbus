## AlfIPbus

AlfIPbus was developed as a temporary Detector Control System (DCS) solution for the Fast Interaction Trigger (FIT) detector in the ALICE experiment. Its primary purpose is to serve as a software translator between the IPbus protocol, used in the previous DCS setup, and the new custom SWT FIT protocol in the updated system. AlfIPbus provides a fast and reliable translation mechanism and is fully compatible with FRED software.

**Current version is not tested for multiple link within one AlfIPbus (as it is not used within FIT)**

## Benchmarks

### Request Execution Time vs Request Size
Each request consists of a read operation, and the execution time recorded for each request size represents the average of 100 measurements. This ensures that the results reflect consistent performance for each data size.

| Size (SWT frames) | Request Execution Time (µs) |
|------------------|----------------------------------|
| 1                | 203.98                           |
| 2                | 232.12                           |
| 4                | 232.48                           |
| 8                | 275.94                           |
| 16               | 318.31                           |
| 32               | 458.70                         |
| 64               | 683.47                           |
| 128              | 1088.61                          |
| 256              | 2069.85                          |
| 512              | 3577.77                          |
| 1024             | 7292.00                         |
| 2048             | 14477.00                         |
| 4096             | 28393.39                         |
| 8192             | 55849.51                         |
| 16384            | 110797.40                        |
| 32768            | 220195.37


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
    - DID_DNS_NODE: Set the DNS node details.
    
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

## Testing
Testing framework was provided by [frun36](https://github.com/frun36) and is available [here](https://github.com/frun36/alf-ipbus-tester).

### In-container-test
The test for developers is available within the `test/in-container-test` directory and can be executed using the `in-container-test.sh` script. Running this test requires the DIM DNS server to be running on your machine. The script builds the AlfIPbus within a container and subsequently builds a container for the testing framework.

The test is conducted using three containers: ALF, Mock, and Generator. The configuration for the test is stored in `test/in-container-test/common-storage/test-configuration.sh`. Logs from all three containers are output to the `test/in-container-test/common-storage/output` directory. If all tests pass, a message will be displayed at the end of the Generator log file.

