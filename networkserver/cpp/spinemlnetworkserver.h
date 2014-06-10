#ifndef SPINEMLNETWORKSERVER_H
#define SPINEMLNETWORKSERVER_H

#include <QObject>

#include <QTcpServer>
#include <QTcpSocket>

#define RESP_DATA_NUMS 31
#define RESP_DATA_SPIKES 32
#define RESP_DATA_IMPULSES 33
#define RESP_HELLO 41
#define RESP_RECVD 42
#define RESP_ABORT 43
#define RESP_FINISHED 44
#define AM_SOURCE 45
#define AM_TARGET 46
#define NOT_SET 99

enum dataTypes {
    ANALOG,
    EVENT,
    IMPULSE
};

class spineMLNetworkServer;

class spineMLNetworkServer : public QObject {
Q_OBJECT

public:
    spineMLNetworkServer(QTcpServer * server, QObject *parent=0);
    ~spineMLNetworkServer() {}

    bool getClient();
    bool connectServer();
    bool handShake();
    bool sendDataType(dataTypes dataType);
    dataTypes recvDataType(bool &ok);
    bool sendSize(int size);
    int recvSize(bool &ok);
    bool sendData(char * ptr, int size);
    bool sendDataConfirm();
    bool recvData(char * data, int size);
    bool disconnectServer();
    bool isSource();
    bool isTarget();
    bool isConnected();
    bool isDataAvailable();
    bool hasSent();
    QTcpServer * server;

    dataTypes dataType;
    int size;

private:
    QTcpSocket * connection;
    char returnVal;
    char sendVal;
    char serverType;
    int n;
    bool has_sent;

public slots:
    void readyToRead();

};



#endif // SPINEMLNETWORKSERVER_H
