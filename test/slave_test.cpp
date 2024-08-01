#include"IPbusSlave.h"
#include"Memory.h"

class Registers : public ipbus::Memory
{
    public:
    bool dataRead(uint32_t address, size_t words, uint32_t* out) const override
    {
        memcpy(out, buffer + address, words*sizeof(uint32_t));
        return true;
    }
    bool dataWrite(uint32_t address, size_t words, const uint32_t* in) override
    {
        memcpy(buffer + address, in, words*sizeof(uint32_t));
        return true;
    }
    void lock() override
    {

    }
    void unlock() override
    {

    }

    size_t getSize() const override
    {
        return size;
    }

    private:
    uint32_t buffer[2048];
    size_t size{2048};
};  


int main()
{
    boost::asio::io_context io;
    Registers reg;
    ipbus::IPbusSlave slave(io, &reg, 50001);
    while(true) io.run();
    
    return 0;
}