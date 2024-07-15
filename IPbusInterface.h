#ifndef IPBUSINTERFACE_H
#define IPBUSINTERFACE_H

#include <QtNetwork/QtNetwork>
#include<QtNetwork/QUdpSocket>
#include <QMutex>
#include<thread>
#include "IPbusControlPacket.h"
#include"boost/asio.hpp"

using namespace boost::asio;

/**
 *  @brief An abstract class that provides communication mechanisms through the IPbus protocol
 * 
*/
class IPbusTarget: public QObject {
    Q_OBJECT
    const quint16 localport;
    /** @brief Handles communication via UDP*/
    QUdpSocket *qsocket = new QUdpSocket(this);
    /** @brief Packet used in the connection diagnostic*/
    const StatusPacket statusRequest;
    /** @brief Response from the remote site involved in the connection diagnostic*/
    StatusPacket statusResponse;
    QMutex mutex;                                
    const int timeout_ms = 99;

public:
    /** @brief IP address of the remote site */
    QString IPaddress;
    bool isOnline = false;
    QTimer *updateTimer = new QTimer(this);
    quint16 updatePeriod_ms = 1000;

    IPbusTarget(QString address = "172.20.75.180", quint16 lport = 0) : localport(lport), IPaddress(address)  {
        qRegisterMetaType<errorType>("errorType");
        qRegisterMetaType<QAbstractSocket::SocketError>("socketError");
        updateTimer->setTimerType(Qt::PreciseTimer);
        connect(updateTimer, &QTimer::timeout, this, [=]() { if (isOnline) sync(); else checkStatus(); });
        connect(this, &IPbusTarget::error, updateTimer, &QTimer::stop);
        qsocket->setProxy(QNetworkProxy::NoProxy);
        if (!qsocket->bind(QHostAddress::AnyIPv4, localport)) qsocket->bind(QHostAddress::AnyIPv4);
        updateTimer->start(updatePeriod_ms);
    }

    /** 
     * @brief Initializes read transaction
     * @details Intializes read transaction from 
    */
    quint32 readRegister(quint32 address) {
        IPbusControlPacket p; //connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(ipread, address, nullptr, 1);
        TransactionHeader *th = p.transactionsList.last().responseHeader;
        return transceive(p, false) && th->InfoCode == 0 ? quint32(*++th) : 0xFFFFFFFF;
    }

signals:
    /// @brief Emitted when a socket error occurs
    void error(QString, errorType);

    /// @brief Emitted when there is no response at the socket, or the response is invalid
    void noResponse(QString message = "no response");

    /// @brief Emitted when IPbus status is confirmed to be OK
    void IPbusStatusOK();

protected:
    /**
     * @brief Sends IPbusControlPacket `p` and waits for a response
     * 
     * - Checks if IPbusTarget is online - `false` if not
     * - Checks if request is empty - `true` if yes, does nothing
     * - Writes to `qsocket` - `false` if error
     * - Reads from `qsocket` - `false` if error or response incorrect
     * 
     * @param[in,out] p The control packet being sent. After response is received it is also stored within `p`
     * @param shouldResponseBeProcessed indicates whether `processResponse` should be called to verify the response
     * 
     * @return `true` if the operation was successful, `false` otherwise
    */
    bool transceive(IPbusControlPacket &p, bool shouldResponseBeProcessed = true) { //send request, wait for response, receive it and check correctness
        if (!isOnline) return false;
        if (p.requestSize <= 1) {
            std::cerr <<"Empty request"; //not a logicError anymore, just nothing to do
            return true;
        }
        QMutexLocker ml(&mutex);
        qint32 n = qint32(qsocket->write((char *)p.request, p.requestSize * wordSize));
        if (n < 0) {
            emit error("Socket write error: " + qsocket->errorString(), networkError);
            return false;
        } else if (n != p.requestSize * wordSize) {
            emit error("Sending packet failed", networkError);
            return false;
        } else if (!qsocket->waitForReadyRead(timeout_ms) && !qsocket->hasPendingDatagrams()) {
            isOnline = false;
            emit noResponse();
            return false;
        }
        n = qsocket->readDatagram((char *)p.response, qsocket->pendingDatagramSize());
        if (n == 64 && p.response[0] == statusRequest.header) { //late status response received
            if (!qsocket->hasPendingDatagrams() && !qsocket->waitForReadyRead(timeout_ms) && !qsocket->hasPendingDatagrams()) {
                isOnline = false;
                emit noResponse();
                return false;
            }
            n = qsocket->readDatagram((char *)p.response, qsocket->pendingDatagramSize());
        }
        if (n == 0) {
            emit error("empty response, no IPbus target on " + IPaddress, networkError);
            return false;
        } else if (n / wordSize > p.responseSize || p.response[0] != p.request[0] || n % wordSize > 0) {
            emit error(QString::asprintf("incorrect response (%d bytes)", n), networkError);
            return false;
        } else {
            p.responseSize = quint16(n / wordSize); //response can be shorter then expected if a transaction wasn't successful
            bool result = shouldResponseBeProcessed ? p.processResponse() : true;
            p.reset();
            return result;
        }
    }

public slots:
    /// @brief Attempts to reconnect to the socket
    void reconnect() {
        if (qsocket->state() == QAbstractSocket::ConnectedState) {
            qsocket->disconnectFromHost();
        }
        qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
        if (!qsocket->waitForConnected(500) && qsocket->state() != QAbstractSocket::ConnectedState) {
            isOnline = false;
            emit noResponse();
            return;
        }
        if (!updateTimer->isActive()) updateTimer->start(updatePeriod_ms);
        checkStatus();
    }

    /// @brief Checks the socket status, sets `IPbusTarget::isOnline` accordingly and emits the proper signal
    void checkStatus() {
        qsocket->write((char *)&statusRequest, sizeof(statusRequest));
        if (!qsocket->waitForReadyRead(timeout_ms) && !qsocket->hasPendingDatagrams()) {
            isOnline = false;
            emit noResponse();
        } else {
            qint32 n = qsocket->read((char *)&statusResponse, qsocket->pendingDatagramSize());
            if (n != sizeof (statusResponse) || statusResponse.header != statusRequest.header) {
                isOnline = false;
                emit noResponse(QString::asprintf("incorrect response (%d bytes). No IPbus?", n));
            } else {
                isOnline = true;
                emit IPbusStatusOK();
            }
        }
    }

    virtual void sync() =0;

    /// @brief Writes a register using IPbus
    /// @param address address of the register
    /// @param data new register value
    /// @param syncOnSuccess whether `IPbus::sync` is to be called upon success
    void writeRegister(quint32 address, quint32 data, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(ipwrite, address, &data, 1);
        if (transceive(p) && syncOnSuccess) sync();
    }

    /// @brief Sets a single bit of a register using IPbus
    /// @param n bit position (0 - LSB)
    /// @param address address of the register
    /// @param syncOnSuccess whether `IPbus::sync` is to be called upon success
    void setBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(RMWbits, address, p.masks(0xFFFFFFFF, 1 << n));
        if (transceive(p) && syncOnSuccess) sync();
    }

    /// @brief Clears a single bit of a register using IPbus
    /// @param n bit position (0 - LSB)
    /// @param address address of the register
    /// @param syncOnSuccess whether `IPbus::sync` is to be called upon success
    void clearBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(RMWbits, address, p.masks(~(1 << n), 0x00000000));
        if (transceive(p) && syncOnSuccess) sync();
    }

    /// @brief Writes a n-bit block in a register
    /// @param address Address of the register
    /// @param data Values for the bits to be set - gets shifted by `shift`
    /// @param nbits Number of bits to be changed (`mask = (1 << nbits) - 1`)
    /// @param shift Shift relative to the LSB (`mask << shift`)
    /// @param syncOnSuccess whether `IPbus::sync` is to be called upon success
    void writeNbits(quint32 address, quint32 data, quint8 nbits = 16, quint8 shift = 0, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addNBitsToChange(address, data, nbits, shift);
        if (transceive(p) && syncOnSuccess) sync();
    }
};

#endif // IPBUSINTERFACE_H
