#include"SWTelectronics.h"
#include<thread>
#include<chrono>
int main(int argc, const char** argv)
{
 boost::asio::io_context io;
 SWTelectronics comm(io);
 comm.stop_timer();
 uint8_t bytes[10];
 std::string message = "300000000000";
 message += argv[1];
 for(int i = 0; i < 10; i++)
 {
  bytes[i] = string_to_byte(message[19-2*i-1], message[19-2*i]);
 }
 std::cout << message << std::endl;
std::this_thread::sleep_for(std::chrono::seconds(5));
 comm.process_request(bytes);
return 0;

}
