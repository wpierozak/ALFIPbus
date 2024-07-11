/**
 * @file IPbusControlPacket.h
 * 
 * @brief Contains the definition of an IPbus control packet (see the [IPbus specification](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf) section 3)
*/

#ifndef IPBUSCONTROLPACKET_H
#define IPBUSCONTROLPACKET_H
#include "IPbusHeaders.h"
#include <QObject>
#include <QDateTime>
#include"SWT.h"

/** 
 *  @brief Maximum size of a single packet specified in words
 *  @details Packet size is limited by the maximum transmission unit (1500 bytes) of the ethernet used in the communication with PMs.
*/
const quint16 maxPacket = 368; //368 words, limit from ethernet MTU of 1500 bytes
enum errorType {networkError = 0, IPbusError = 1, logicError = 2};
static const char *errorTypeName[3] = {"Network error" , "IPbus error", "Logic error"};

/** 
 *  @brief A class responsible for creating a single IPbus packet and checking the corectness of the response
 *  @details IPbusControlPacket stacks one or more IPbus transactions into a single packet. See the [IPbus specification](https://ipbus.web.cern.ch/doc/user/html/_downloads/d251e03ea4badd71f62cffb24f110cfa/ipbus_protocol_v2_0.pdf) section 3
*/
class IPbusControlPacket : public QObject {
    Q_OBJECT
public:
    /** \brief List of transactions that will be sent  */
    QList<Transaction> transactionsList;
    /** \brief Size of the request specified in words */            
    quint16 requestSize = 1,
    /** \brief Size of the response specified in words */ 
            responseSize = 1;
    /** \brief Buffer where the request is stored */    
    quint32 request[maxPacket], 
    /** \brief Buffer where the response will be saved */    
            response[maxPacket];
    /** \brief Temporary data */
    quint32 dt[2];

/**
 *  @brief Default constructor
 *  @details Default constructor. Initialization of an IPbusControlPacker object involves two routines:
 *  - Creation of the packet header and placement of header at the beginnig of the packer buffer
 *  - Creation of the pair of signal and slot (Qt) - function error is set as singal emitter and debugPrint is set as a slot (handler)
*/
    IPbusControlPacket() {
        request[0] = PacketHeader(control, 0);
        connect(this, &IPbusControlPacket::error, this, &IPbusControlPacket::debugPrint);
    }
    ~IPbusControlPacket() { this->disconnect(); }

/**
 *  @brief A method used in debugging. It prints text containg request and response messages
 * 
 *  @details `debugPrint` prints debug information in the following manner: @n
 *  yyyy-MM-dd hh:mm:ss.zzz + st @n
 *  request: @n
 *  <request printed in the hexadecimal format> @n
 *  response: @n
 *  <response printed in the hexadecimal format> @n
 *  
 *  @param st - Message to be printed along with response and request messages
*/
    void debugPrint(QString st) {
        qDebug(qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + st));
        qDebug("request:");
        for (quint16 i=0; i<requestSize; ++i)  qDebug("%08X", request[i]);
        qDebug("        response:");
        for (quint16 i=0; i<responseSize; ++i) qDebug("        %08X", response[i]);
    }

/**
 * @brief A method that creates a mask used in the RMWbit transaction
 * 
 * @details RMWbit operation is performed according to the equation: X = (X & A)\B, where X is a word (register) on which operation is 
 * performed. Mask is saved in the dt field, AND term is saved as the first element, OR term is saved as the second.
 * 
 * @param mask0 - AND term of a RMWbit transaction
 * @param mask1 - OR term of a RMWbit transaction
 * 
 * @return Pointer to the mask (dt field)
*/
    quint32 *masks(quint32 mask0, quint32 mask1) { //for convinient adding RMWbit transaction
        dt[0] = mask0; //for writing 0's: AND term
        dt[1] = mask1; //for writing 1's: OR term
        return dt;
    }

/** 
 *  @brief Adds transaction to the packet
 * 
 *  @details addTransaction saves transaction in two formats: within transactionsList as Transaction object and within the request buffer in a format appropriate for IPbus communication.
 *  Transactions within a packet are stored in the following manner (values in [] represent bit numbers):
 *  - [0-31] - packet header
 *  - [32-63] - transaction header
 *  - [64-95] - destination address
 *  - [96-...] - data
 *  - [x-x+32] - next transaction header
 *  - ...
 *  Number of transactions within one packet is limited by the maximum packet size.
 * 
 *  @param type Type of transaction
 *  @param address Address of a memory location at the remote site on which operation will be performed
 *  @param data Address of a memory block of data passsed to the transaction
 *  @param nWords Size of a data block specified in words
*/
    void addTransaction(TransactionType type, quint32 address, quint32 *data, quint8 nWords = 1) {
        Transaction currentTransaction;
        request[requestSize] = TransactionHeader(type, nWords, transactionsList.size());
        currentTransaction.requestHeader = (TransactionHeader *)(request + requestSize++);
        request[requestSize] = address;
        currentTransaction.address = request + requestSize++;
        currentTransaction.responseHeader = (TransactionHeader *)(response + responseSize++);
        switch (type) {
            case                ipread:
            case nonIncrementingRead:
            case   configurationRead:
                currentTransaction.data = data;
                responseSize += nWords;
                break;
            case                ipwrite:
            case nonIncrementingWrite:
            case   configurationWrite:
                currentTransaction.data = request + requestSize;
                for (quint8 i=0; i<nWords; ++i) request[requestSize++] = data[i];
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
                emit error("unknown transaction type", IPbusError);
        }
        if (requestSize > maxPacket || responseSize > maxPacket) {
            emit error("packet size exceeded", IPbusError);
            return;
        } else transactionsList.append(currentTransaction);
    }

/**
 * @brief Adds a writing transaction to the packet
 * 
 * @param address Address on the remote site where a word should be written to
 * @param value The word to be written
*/
    void addWordToWrite(quint32 address, quint32 value) { addTransaction(ipwrite, address, &value, 1); }

    /// @brief Adds a transaction changing a n-bit block in a register
    /// @param address Address of the register
    /// @param data Values for the bits to be set - gets shifted by `shift`
    /// @param nbits Number of bits to be changed (`mask = (1 << nbits) - 1`)
    /// @param shift Shift relative to the LSB (`mask << shift`)
    void addNBitsToChange(quint32 address, quint32 data, quint8 nbits, quint8 shift = 0) {
        if (nbits == 32) { addTransaction(ipwrite, address, &data, 1); return; }
        quint32 mask = (1 << nbits) - 1; //e.g. 0x00000FFF for nbits==12
        addTransaction(RMWbits, address, masks( ~quint32(mask << shift), quint32((data & mask) << shift) ));
    }

/**
 * @brief Check transactions successfulness and copy read data to destination
 * 
 * @return `true` is returned if all transactions were successful, otherwise `false` is returned
*/
    bool processResponse() { 
        for (quint16 i=0; i<transactionsList.size(); ++i) {
            TransactionHeader *th = transactionsList.at(i).responseHeader;
            if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) {
                emit error(QString::asprintf("unexpected transaction header: %08X, expected: %08X", *th, *transactionsList.at(i).requestHeader & 0xFFFFFFF0), IPbusError);
                return false;
            }
            if (th->Words > 0) switch (th->TypeID) {
                case                ipread:
                case nonIncrementingRead:
                case   configurationRead: {
                    quint32 wordsAhead = response + responseSize - (quint32 *)th - 1;
                    if (th->Words > wordsAhead) { //response too short to contain nWords values
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (quint32 *)th + 1, wordsAhead * wordSize);
                        emit successfulRead(wordsAhead);
                        if (th->InfoCode == 0) emit error(QString::asprintf("read transaction from %08X truncated: %d/%d words received", *transactionsList.at(i).address, wordsAhead, th->Words), IPbusError);
                        return false;
                    } else {
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (quint32 *)th + 1, th->Words * wordSize);
                        emit successfulRead(th->Words);
                    }
                    break;
                }
                case RMWbits:
                case RMWsum :
                    if (th->Words != 1) {
                        emit error("wrong RMW transaction", IPbusError);
                        return false;
                    }
                    emit successfulRead(1);
                    /* fall through */ //[[fallthrough]];
                case                ipwrite:
                case nonIncrementingWrite:
                case   configurationWrite:
                    emit successfulWrite(th->Words);
                    break;
                default:
                    emit error("unknown transaction type", IPbusError);
                    return false;
            }
            if (th->InfoCode != 0) {
                emit error(th->infoCodeString() + QString::asprintf(", address: %08X", *transactionsList.at(i).address + th->Words), IPbusError);
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

signals:
    /// @brief Emitted on IPbus error
    void error(QString, errorType);
    
    /// @brief Emitted upon successfully reading `nWords` 
    void successfulRead(quint8 nWords);
    
    /// @brief Emitted upon successfully writing `nWords`
    void successfulWrite(quint8 nWords);
};


#endif // IPBUSCONTROLPACKET_H
