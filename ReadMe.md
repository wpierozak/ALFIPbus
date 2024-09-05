## Building
```
git submodule update --recursive --init
mkdir build
cd build
cmake3 ..
cmake3 --build .
```

## Steps to Configure and Enable alfipbus service

1. **Copy the Service File**  
   First, copy the `alfipbus.service` file to the systemd unit file directory (the default is `/etc/systemd/system`). You can do this with the following command:
   
    ```  
    bash
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
