#include<cstdint>

#ifndef IPBUS_MEMORY
#define IPBUS_MEMORY
namespace ipbus
{

    class Memory
    {
        public:
        virtual bool dataRead(uint32_t address, size_t words, uint32_t* out) const = 0;
        virtual bool dataWrite(uint32_t address, size_t words, const uint32_t* in) = 0;
        virtual void lock() = 0;
        virtual void unlock() = 0;
        virtual size_t getSize() const = 0;
    };
}
#endif