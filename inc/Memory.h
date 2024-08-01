#include<cstdint>

namespace ipbus
{
    enum InfoCode{Response=0x0,
                    BadHeader=0x1,
                    ErrorRead=0x4,
                    ErrorWrite=0x5,
                    TimeoutRead=0x6,
                    TimeoutWrite=0x7,
                    Request=0xf};
    class Memory
    {
        public:
        virtual InfoCode dataRead(uint32_t address, size_t words, uint32_t* out) const = 0;
        virtual InfoCode dataWrite(uint32_t address, size_t words, const uint32_t* in) = 0;
        virtual size_t getSize() const = 0;
    };
}