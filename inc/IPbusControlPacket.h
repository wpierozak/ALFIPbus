
#ifndef IPBUSCONTROLPACKET_H
#define IPBUSCONTROLPACKET_H
#include "IPbusHeaders.h"
#include<vector>
#include<iostream>

namespace IPbus
{

const uint16_t maxPacket = 368; //368 words, limit from ethernet MTU of 1500 bytes
enum errorType {networkError = 0, IPbusError = 1, logicError = 2};
static const char *errorTypeName[3] = {"Network error" , "IPbus error", "Logic error"};

class IPbusControlPacket {
  
public:
    /** \brief List of transactions that will be sent  */
    std::vector<Transaction> transactionsList;
    /** \brief Size of the request specified in words */            
    uint16_t requestSize = 1,
    /** \brief Size of the response specified in words */ 
            responseSize = 1;
    /** \brief Buffer where the request is stored */    
    uint32_t request[maxPacket], 
    /** \brief Buffer where the response will be saved */    
            response[maxPacket];
    uint32_t dt[2];


    IPbusControlPacket() {
        request[0] = PacketHeader(control, 0);
    }
    ~IPbusControlPacket() { }


    void debugPrint(std::string st);


    uint32_t *masks(uint32_t mask0, uint32_t mask1);

    void addTransaction(TransactionType type, uint32_t address, uint32_t *data, uint8_t nWords = 1);

/**
 * @brief Adds a writing transaction to the packet
 * 
 * @param address Address on the remote site where a word should be written to
 * @param value The word to be written
*/
    void addWordToWrite(uint32_t address, uint32_t value);

    /// @brief Adds a transaction changing a n-bit block in a register
    /// @param address Address of the register
    /// @param data Values for the bits to be set - gets shifted by `shift`
    /// @param nbits Number of bits to be changed (`mask = (1 << nbits) - 1`)
    /// @param shift Shift relative to the LSB (`mask << shift`)
    void addNBitsToChange(uint32_t address, uint32_t data, uint8_t nbits, uint8_t shift = 0);
/**
 * @brief Check transactions successfulness and copy read data to destination
 * 
 * @return `true` is returned if all transactions were successful, otherwise `false` is returned
*/
    bool processResponse();
/**
 * @brief resets packet data
 * 
 * @details reset method clears transactionList and resets request and response sizes to 1.
*/
    void reset();
};

}

#endif // IPBUSCONTROLPACKET_H
