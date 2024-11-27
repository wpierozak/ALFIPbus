# DimRpcParallel

## Description

### Class: DimRpcParallel

### Constructors:<br />
* public DimRpcParallel(char* name, char* format_in, char* format_out, int bank); First parameter is the Service Name. The format parameters specifies the contents of the data to be received (format_in) and to be sent in response (format_out) in the in the form described here. Bank can be any integer value, services registered within the same bank are executed sequentially, different banks are executed in parallel.<br />
### Destructors:<br />
* public ~DimRpcParallel();<br />
### Public Functions:<br />
Handler: Gets Called when an RPC is requested by a Client (DimRpcInfo) <br />
* virtual void rpcHandler(); // Has to be provided by the user.<br /><br />
Get Methods: To be used inside rpcHandler in order to get the data received from the client <br /><br />
* int getInt(); // Get an Integer
* float getFloat();
* double getDouble();
* short getShort();
* longlong getLonglong();
* const char* getString();
* int getSize(); //Get the size of the data (for complex types)
* void* getData(); //Get the data (for complex types)<br /><br />
Set Methods: To be used inside rpcHandler in order to send the result back to the client <br /><br />
* int setData(int& value); //Send back an Integer
* int setData(float& value);
* int setData(double& value);
* int setData(short& value);
* int setData(longlong& value);
* int setData(const char* string);
* int setData(void* data, int size); //Send back complex data

## Compilation

g++ -Wall -Werror -I /home/flp/slc7_x86-64/DimRpcParallel/latest/include -I /home/flp/slc7_x86-64/dim/latest server.cpp -o test -L /home/flp/slc7_x86-64/DimRpcParallel/latest/lib -L /home/flp/slc7_x86-64/dim/latest -lDimRpcParallel -ldim -lpthread
