
#ifndef IPBUSCONTROLPACKET_H
#define IPBUSCONTROLPACKET_H
#include "IPbusHeaders.h"
#include<vector>
#include<iostream>


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
    /** \brief Temporary data */
    uint32_t dt[2];


    IPbusControlPacket() {
        request[0] = PacketHeader(control, 0);
        //connect(this, &IPbusControlPacket::error, this, &IPbusControlPacket::debugPrint);
    }
    ~IPbusControlPacket() { }


    void debugPrint(std::string st) {
        //qDebug(qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + st));
        std::cerr<< "request:\n";
        for (uint16_t i=0; i<requestSize; ++i)  std::cerr<< std::hex << request[i] <<  std::endl;
        std::cerr<< "\t\tresponse:" << std::endl;
        for (uint16_t i=0; i<responseSize; ++i) std::cerr<< "\t\t" << std::hex <<  response[i] << std::endl;
    }


    uint32_t *masks(uint32_t mask0, uint32_t mask1) { //for convinient adding RMWbit transaction
        dt[0] = mask0; //for writing 0's: AND term
        dt[1] = mask1; //for writing 1's: OR term
        return dt;
    }


    void addTransaction(TransactionType type, uint32_t address, uint32_t *data, uint8_t nWords = 1) {
        Transaction currentTransaction;
        request[requestSize] = TransactionHeader(type, nWords, transactionsList.size());
        currentTransaction.requestHeader = (TransactionHeader *)(request + requestSize++);
        request[requestSize] = address;
        currentTransaction.address = request + requestSize++;
        currentTransaction.responseHeader = (TransactionHeader *)(response + responseSize++);
        switch (type) {
            case                read:
            case nonIncrementingRead:
            case   configurationRead:
                currentTransaction.data = data;
                responseSize += nWords;
                break;
            case                write:
            case nonIncrementingWrite:
            case   configurationWrite:
                currentTransaction.data = request + requestSize;
                for (uint8_t i=0; i<nWords; ++i) request[requestSize++] = data[i];
                break;
            case RMWbits:
                request[requestSize++] = data[0]; //AND term
                request[requestSize++] = data[1]; // OR term
                currentTransaction.data = response + responseSize++;
                break;
            case RMWsum:
                request[requestSize++] = *data; //addend
                currentTransaction.data = response + responseSize++;
                break;
            default:
                debugPrint("unknown transaction type");
        }
        if (requestSize > maxPacket || responseSize > maxPacket) {
            debugPrint("packet size exceeded");
            return;
        } else transactionsList.push_back(currentTransaction);
    }

/**
 * @brief Adds a writing transaction to the packet
 * 
 * @param address Address on the remote site where a word should be written to
 * @param value The word to be written
*/
    void addWordToWrite(uint32_t address, uint32_t value) { addTransaction(write, address, &value, 1); }

    /// @brief Adds a transaction changing a n-bit block in a register
    /// @param address Address of the register
    /// @param data Values for the bits to be set - gets shifted by `shift`
    /// @param nbits Number of bits to be changed (`mask = (1 << nbits) - 1`)
    /// @param shift Shift relative to the LSB (`mask << shift`)
    void addNBitsToChange(uint32_t address, uint32_t data, uint8_t nbits, uint8_t shift = 0) {
        if (nbits == 32) { addTransaction(write, address, &data, 1); return; }
        uint32_t mask = (1 << nbits) - 1; //e.g. 0x00000FFF for nbits==12
        addTransaction(RMWbits, address, masks( ~uint32_t(mask << shift), uint32_t((data & mask) << shift) ));
    }

/**
 * @brief Check transactions successfulness and copy read data to destination
 * 
 * @return `true` is returned if all transactions were successful, otherwise `false` is returned
*/
    bool processResponse() { 
        for (uint16_t i=0; i<transactionsList.size(); ++i) {
            TransactionHeader *th = transactionsList.at(i).responseHeader;
            if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) {
                std::string message = "unexpected transaction header: " + std::to_string(*th) + ", expected: " + std::to_string(*transactionsList.at(i).requestHeader & 0xFFFFFFF0);
                debugPrint(message);
                return false;
            }
            if (th->Words > 0) switch (th->TypeID) {
                case                read:
                case nonIncrementingRead:
                case   configurationRead: {
                    uint32_t wordsAhead = response + responseSize - (uint32_t *)th - 1;
                    if (th->Words > wordsAhead) { //response too short to contain nWords values
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (uint32_t *)th + 1, wordsAhead * wordSize);
                        //emit successfulRead(wordsAhead); !!! 
                        if (th->InfoCode == 0) 
                        {
                            std::string message = "read transaction from " + std::to_string(*transactionsList.at(i).address) + " truncated " + std::to_string(wordsAhead) + "words received: " + std::to_string(th->Words);
                            debugPrint(message);
                        }
                        return false;
                    } else {
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (uint32_t *)th + 1, th->Words * wordSize);
                        //emit successfulRead(th->Words); !!!
                    }
                    break;
                }
                case RMWbits:
                case RMWsum :
                    if (th->Words != 1) {
                        debugPrint("wrong RMW transaction");
                        return false;
                    }
                    //emit successfulRead(1); !!!
                    /* fall through */ //[[fallthrough]];
                case                write:
                case nonIncrementingWrite:
                case   configurationWrite:
                    //emit successfulWrite(th->Words);
                    break;
                default:
                    debugPrint("unknown transaction type");
                    return false;
            }
            if (th->InfoCode != 0) {
                std::string message = th->infoCodeString() + ", address: " + std::to_string(*transactionsList.at(i).address + th->Words);
                debugPrint(message);
                return false;
            }
        }
        return true;
    }
/**
 * @brief resets packet data
 * 
 * @details reset method clears transactionList and resets request and response sizes to 1.
*/
    void reset() {
        transactionsList.clear();
        requestSize = 1;
        responseSize = 1;
    }

};


#endif // IPBUSCONTROLPACKET_H
