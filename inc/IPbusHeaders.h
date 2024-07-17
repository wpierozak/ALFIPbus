/**
 * @file IPbusHeaders.h
 *
 * Representation of the packet and transaction headers. Compare with the [IPbus specification](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf).
 *
 * # Terminology
 * ## IPbus transaction
 * In the IPbus specification - An individual IPbus request or response
 *
 * Here - stores an individual IPbus request and its response (after it comes)
 *
 * ## IPbus packet
 * An IPbus packet header (32b) + one or more individual IPbus transactions, which together form the payload of the transport protocol
*/

#ifndef IPBUSHEADERS_H
#define IPBUSHEADERS_H

#include<cstdint>
#include<cstring>
#include<string>
#include <climits>

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

/**
 * @brief Size of one word in bytes
*/
const uint8_t wordSize = sizeof(uint32_t); //4 bytes

/**
 * @brief Packet Type values of an IPbus header
 *
 * Supported values:
 * - 0x0; Direction: Both; Control packet (i.e. contains IPbus transactions)
 * - 0x1; Direction: Both; Status packet
 * - 0x2; Direction: Request; Resend request packet
 * - 0x3-0xf - reserved
*/
enum PacketType {control = 0, status = 1, resend = 2};

/**
 * @brief Representation of the packet header
 *
 * See the [IPbus specification](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf) section 2 for detailed descriptions
 * and `IPbusHeaders.h` for terminology
 *
 * Contains bit fields representing the respective fields of an IPbus packet header - note the little endianness
*/
struct PacketHeader {
    uint32_t PacketType      :  4,
        ByteOrder       :  4,
        PacketID        : 16,
        Rsvd            :  4,
        ProtocolVersion :  4;

    /// @brief Creates a `PacketHeader` for a packet of type `t` and ID = `id`, with byte order `0xf` and protocol version 2
    PacketHeader(enum PacketType t = status, uint16_t id = 0) {
        PacketType = t;
        ByteOrder = 0xf;
        PacketID = id;
        Rsvd = 0;
        ProtocolVersion = 2;
    }

    /// @brief Constructs the packet header from a raw `uint32_t` value
    PacketHeader(const uint32_t &word) {memcpy(this, &word, wordSize);}

    /// @brief Converts the header to a raw `uint32_t` value
    operator uint32_t() const {return *reinterpret_cast<const uint32_t *>(this);}
};


enum TransactionType {
    ipread                  = 0,
    ipwrite                 = 1,
    nonIncrementingRead   = 2,
    nonIncrementingWrite  = 3,
    RMWbits               = 4,
    RMWsum                = 5,
    configurationRead     = 6,
    configurationWrite    = 7
};


struct TransactionHeader {
    uint32_t InfoCode        :  4,
        TypeID          :  4,
        Words           :  8,
        TransactionID   : 12,
        ProtocolVersion :  4;
    TransactionHeader(TransactionType t, uint8_t nWords, uint16_t id = 0) {
        InfoCode = 0xf;
        TypeID = t;
        Words = nWords;
        TransactionID = id;
        ProtocolVersion = 2;
    }
    TransactionHeader(const uint32_t &word) {memcpy(this, &word, wordSize);}
    operator uint32_t() {return *reinterpret_cast<uint32_t *>(this);}
    std::string infoCodeString() {
        switch (InfoCode) {
        case 0x0: return "Successfull request";
        case 0x1: return "Bad header";
        case 0x4: return "IPbus read error";
        case 0x5: return "IPbus write error";
        case 0x6: return "IPbus read timeout";
        case 0x7: return "IPbus write timeout";
        case 0xf: return "outbound request";
        default : return "unknown Info Code";
        }
    }
};

/** @brief A struct containing full information about a single transaction
 *  @details  is represented within the IPbus packet by three components: transaction header (1 word), Address of the memory location on which the operation will be performed (1 word)
 * and a block of data (if any data is required). Data layout is speficic for each kind of transaction. Transaction stores information about request, also after
 * the response is received pointer to the response header will be stored in field responseHeader.
*/
struct Transaction {
    TransactionHeader
        /** Request transaction header describes  */
            *requestHeader,
        /** Address to the response header will be saved here */
        *responseHeader;

    uint32_t
        /** Address of the memory location on which the operation will be performed */
            *address,
        /** Address of the block of data used in the transaction*/
        *data;
};

/** @brief A struct containing definition of the packet used to check the connection
*/
struct StatusPacket {
    PacketHeader header = swap_endian<uint32_t>(uint32_t(PacketHeader(status))); //0x200000F1: {0xF1, 0, 0, 0x20} -> {0x20, 0, 0, 0xF1}
    uint32_t MTU = 0,
        nResponseBuffers = 0,
        nextPacketID = 0;
    uint8_t  trafficHistory[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    uint32_t controlHistory[8] = {0,0,0,0, 0,0,0,0};
};


#endif // IPBUSHEADERS_H
