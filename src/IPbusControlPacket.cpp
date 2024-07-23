#include"IPbusControlPacket.h"

namespace ipbus
{

void IPbusControlPacket::debugPrint(std::string st) 
{
    std::cerr<< "request:\n";
    for (uint16_t i=0; i<requestSize; ++i)  std::cerr<< std::hex << request[i] <<  std::endl;
    std::cerr<< "\t\tresponse:" << std::endl;
    for (uint16_t i=0; i<responseSize; ++i) std::cerr<< "\t\t" << std::hex <<  response[i] << std::endl;
}

uint32_t* IPbusControlPacket::masks(uint32_t mask0, uint32_t mask1) 
{ 
    dt[0] = mask0; //for writing 0's: AND term
    dt[1] = mask1; //for writing 1's: OR term
    return dt;
}

void IPbusControlPacket::addTransaction(TransactionType type, uint32_t address, uint32_t *data, uint8_t nWords) 
{
        Transaction currentTransaction;
        request[requestSize] = TransactionHeader(type, nWords, transactionsList.size());
        currentTransaction.requestHeader = (TransactionHeader *)(request + requestSize++);
        request[requestSize] = address;
        currentTransaction.address = request + requestSize++;
        currentTransaction.responseHeader = (TransactionHeader *)(response + responseSize++);

        switch (type) {
            case                data_read:
            case nonIncrementingRead:
            case   configurationRead:
                currentTransaction.data = data;
                responseSize += nWords;
                break;

            case                data_write:
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

void IPbusControlPacket::addWordToWrite(uint32_t address, uint32_t value) 
{ 
    addTransaction(data_write, address, &value, 1); 
}

void IPbusControlPacket::addNBitsToChange(uint32_t address, uint32_t data, uint8_t nbits, uint8_t shift) 
{
        if (nbits == 32) 
        { 
            addTransaction(data_write, address, &data, 1); return; 
        }
        uint32_t mask = (1 << nbits) - 1;
        addTransaction(RMWbits, address, masks( ~uint32_t(mask << shift), uint32_t((data & mask) << shift) ));
}


bool IPbusControlPacket::processResponse() 
{ 
    for (uint16_t i=0; i<transactionsList.size(); ++i) {
        TransactionHeader *th = transactionsList.at(i).responseHeader;
        if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) 
        {
            std::string message = "unexpected transaction header: " + std::to_string(*th) + ", expected: " + std::to_string(*transactionsList.at(i).requestHeader & 0xFFFFFFF0);
            debugPrint(message);
            return false;
        }
        if (th->Words > 0) switch (th->TypeID) {
            case                data_read:
            case nonIncrementingRead:
            case   configurationRead: 
            {
                uint32_t wordsAhead = response + responseSize - (uint32_t *)th - 1;
                if (th->Words > wordsAhead) 
                { //response too short to contain nWords values
                    if (transactionsList.at(i).data != nullptr) 
                    {
                        memcpy(transactionsList.at(i).data, (uint32_t *)th + 1, wordsAhead * wordSize);
                    }
                    //emit successfulRead(wordsAhead); !!! 
                    if (th->InfoCode == 0) 
                    {
                        std::string message = "read transaction from " + std::to_string(*transactionsList.at(i).address) + " truncated " + std::to_string(wordsAhead) + "words received: " + std::to_string(th->Words);
                        debugPrint(message);
                    }
                    return false;
                } 
                else 
                {
                    if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (uint32_t *)th + 1, th->Words * wordSize);
                    //emit successfulRead(th->Words); !!!
                }
                break;

            }

            case RMWbits:
            case RMWsum :
                if (th->Words != 1)
                {
                    debugPrint("wrong RMW transaction");
                    return false;
                }
                //emit successfulRead(1); !!!
                   
            case data_write:
            case nonIncrementingWrite:
            case configurationWrite:
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

void IPbusControlPacket::reset() 
{
    transactionsList.clear();
    requestSize = 1;
    responseSize = 1;
}


}