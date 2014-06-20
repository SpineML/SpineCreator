#include "spinemlnetworkserver.h"

spineMLNetworkServer::spineMLNetworkServer(QTcpServer * server, QObject *parent) :
    QObject(parent)
{
    this->server = server;
    connection = NULL;
    serverType = NOT_SET;
    has_sent = false;
}

bool spineMLNetworkServer::isSource() {
    return (serverType == AM_SOURCE);
}

bool spineMLNetworkServer::isTarget() {
    return (serverType == AM_TARGET);
}

bool spineMLNetworkServer::isConnected() {
    return (connection->isValid() != NULL);
}

// get the next pending connection and set it up as a new client
bool spineMLNetworkServer::getClient() {

    if (!this->connectServer()) {
        return false;
    }

    qDebug() << "Server connected";

    if (!this->handShake()) {
        this->disconnectServer();
        return false;
    }

    qDebug() << "Done Handshake";

    bool ok;

    this->dataType = this->recvDataType(ok);
    if (!ok) {
        this->disconnectServer();
        return false;
    }

    qDebug() << "got data type";

    this->size = this->recvSize(ok);
    if (!ok) {
        this->disconnectServer();
        return false;
    }

    qDebug() << "got size";

    // for debug
    //connect(this->connection, SIGNAL(readyRead()), this, SLOT(readyToRead()));

    return true;

}

bool spineMLNetworkServer::connectServer() {

    // connect the socket:

    //qDebug() << "connect";

    has_sent = false;

    if (server->isListening() && connection == NULL) {
        connection = server->nextPendingConnection();
        if (connection == NULL) {
            return false;
        } else {
            return true;
        }
    }

    return false;
}

bool spineMLNetworkServer::handShake() {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    qDebug() << "handshake recv";

    while (connection->bytesAvailable() < 1) {
        if (!connection->waitForReadyRead(1000)) {
            qDebug() << "Timeout reading in handshake";
            disconnectServer();
            return false;
        }
    }

    // get type of conn
    n = connection->read(&(returnVal),1);
    if (n < 0) {
        qDebug() << "Error reading in handshake";
        disconnectServer();
        return false;
    }

    if (returnVal == AM_SOURCE) {
        // he's a source so we're a target
        serverType = AM_TARGET;
    } else if (returnVal == AM_TARGET) {
        // he's a target so we're a source
        serverType = AM_SOURCE;
    } else {
        qDebug() << "Error handshaking: bad data " << QString::number(returnVal);
        disconnectServer();
        return false;
    }

    qDebug() << "handshake reply";

    // send reply
    sendVal = RESP_HELLO;
    n = connection->write(&sendVal,1);
    if (n < 0) {
        qDebug() << "Error writing in handshake";
        disconnectServer();
        return false;
    }

    connection->flush();

    return true;
}

bool spineMLNetworkServer::sendDataType(dataTypes dataType) {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    qDebug() << "send datatype";

    // send the data type
    switch (dataType) {
        case ANALOG:
            sendVal = RESP_DATA_NUMS;
            break;
        case EVENT:
            sendVal = RESP_DATA_SPIKES;
            break;
        case IMPULSE:
            sendVal = RESP_DATA_IMPULSES;
            break;
    }

    n = connection->write(&sendVal,1);
    if (n < 0) {
        qDebug() << "Error writing in sendDataType";
        disconnectServer();
        return false;
    }

    connection->flush();

    qDebug() << "datatype reply recv";

    while (connection->bytesAvailable() < 1) {
        if (!connection->waitForReadyRead(1000)) {
            qDebug() << "Timeout reading in sendDataType";
            disconnectServer();
            return false;
        }
    }

    // get reply
    n = connection->read(&(returnVal),1);
    if (n < 0) {
        qDebug() << "Error reading in sendDataType";
        disconnectServer();
        return false;
    }

    if (returnVal == RESP_ABORT) {
        qDebug() << "Aborted by client in sendDataType";
        disconnectServer();
        return false;
    }

    return true;
}

dataTypes spineMLNetworkServer::recvDataType(bool &ok) {

    if (!connection) {
        qDebug() << "No connection";
        return ANALOG;
    }

    ok = true;

    qDebug() << "datatype recv";

    while (connection->bytesAvailable() < 1) {
        if (!connection->waitForReadyRead(1000)) {
            qDebug() << "Timeout reading in recvDataType";
            disconnectServer();
            ok = false;
            return ANALOG;
        }
    }

    // get dataType
    n = connection->read(&(returnVal),1);
    if (n < 0) {
        qDebug() << "Error reading in recvDataType";
        disconnectServer();
        ok = false;
        return ANALOG;
    }

    if (returnVal != RESP_DATA_NUMS && returnVal != RESP_DATA_SPIKES && returnVal != RESP_DATA_IMPULSES){
        qDebug() << "Bad data in recvDataType";
        disconnectServer();
        ok = false;
        return ANALOG;
    }

    qDebug() << "datatype reply send";

    sendVal = RESP_RECVD;

    n = connection->write(&sendVal,1);
    if (n < 0) {
        qDebug() << "Error writing in recvDataType";
        disconnectServer();
        ok = false;
        return ANALOG;
    }

    connection->flush();

    dataTypes dataType;
    switch (returnVal) {
         case RESP_DATA_NUMS:
             dataType = ANALOG;
             break;
         case RESP_DATA_SPIKES:
             dataType = EVENT;
             break;
         case RESP_DATA_IMPULSES:
             dataType = IMPULSE;
             break;
     }

    return (dataTypes) dataType;

}

bool spineMLNetworkServer::sendSize(int size) {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    qDebug() << "size send";

    // send size
    n = connection->write((char *) &size,sizeof(int));
    if (n < 0) {
        qDebug() << "Error writing in sendSize";
        disconnectServer();
        return false;
    }

    connection->flush();

    qDebug() << "size reply recv";

    while (connection->bytesAvailable() < 1) {
        if (!connection->waitForReadyRead(1000)) {
            qDebug() << "Timeout reading in sendSize";
            disconnectServer();
            return false;
        }
    }

    // get reply
    n = connection->read(&(returnVal),1);
    if (n < 0) {
        qDebug() << "Error reading in sendSize";
        disconnectServer();
        return false;
    }

    if (returnVal == RESP_ABORT) {
        qDebug() << "Aborted by client in sendSize";
        disconnectServer();
        return false;
    }

    return true;

}

int spineMLNetworkServer::recvSize(bool &ok) {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    qDebug() << "size recv";

    int size;

    while (connection->bytesAvailable() < sizeof(size)) {
        if (!connection->waitForReadyRead(1000)) {
            qDebug() << "Timeout reading in recvSize";
            disconnectServer();
            return false;
        }
    }

    // get size
    n = connection->read((char *) &(size),sizeof(size));
    if (n < 0) {
        qDebug() << "Error reading in recvSize";
        disconnectServer();
        ok = false;
        return 0;
    }

    if (size < 0) {
        qDebug() << "Bad data in recvSize";
        disconnectServer();
        ok = false;
        return 0;
    }

    qDebug() << "size reply send";

    sendVal = RESP_RECVD;

    n = connection->write(&sendVal,1);
    if (n < 1) {
        qDebug() << "Error writing in recvSize";
        disconnectServer();
        ok = false;
        return 0;
    }

    connection->flush();

    return size;

}

bool spineMLNetworkServer::hasSent() {
    return this->has_sent;
}

bool spineMLNetworkServer::sendData(char * ptr, int size) {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    // send data
    int sent_bytes = 0;
    while (sent_bytes < sizeof(double)*size)
        sent_bytes += connection->write(ptr+sent_bytes,sizeof(double)*size);
    if (n < 0) {
        qDebug() << "Error writing in sendData";
        disconnectServer();
        return false;
    }

    connection->waitForBytesWritten();

    has_sent = true;

    return true;

}

bool spineMLNetworkServer::sendDataConfirm() {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }

    if (!has_sent) {
        // this stops the issue where we can connect in between sendData and this function sometimes!
        return true;
    }

    // needs to be separated due to threading

    connection->flush();

    // if we haven't already bytes in the buffer, then wait until there are some
    if (connection->bytesAvailable() == 0) {
        if (!connection->waitForReadyRead(30)) {
            return false;
        }
    }

    // get reply
    n = connection->read(&(returnVal),1);
    if (n < 0) {
        qDebug() << "Error reading in sendData";
        disconnectServer();
        return false;
    }

    if (returnVal == RESP_ABORT) {
        qDebug() << "Aborted by client in sendData";
        disconnectServer();
        return false;
    }

    has_sent = false;

    return true;
}

bool spineMLNetworkServer::recvData(char * data, int size) {

    if (!connection) {
        qDebug() << "No connection";
        return false;
    }
    connection->flush();

    // if we haven't already bytes in the buffer, then wait until there are some
    if (connection->bytesAvailable() == 0) {
        if (!connection->waitForReadyRead(30)) {
            qDebug() << "no data";
            return false;
        }
    }

    // get data
    n = connection->read(data,sizeof(double)*size);
    if (n < 0) {
        qDebug() << "Error reading in recvData";
        disconnectServer();
        return false;
    }

    if (size < 0) {
        qDebug() << "Bad data in recvData";
        disconnectServer();
        return false;
    }

    sendVal = RESP_RECVD;

    n = connection->write(&sendVal,1);
    if (n < 0) {
        qDebug() << "Error writing in recvData";
        disconnectServer();
        return false;
    }

    connection->waitForBytesWritten();

    return true;
}

bool spineMLNetworkServer::isDataAvailable() {

    if (connection == NULL) {
        qDebug() << "No connection";
        return false;
    }

    return (connection->bytesAvailable() > 0);
}

bool spineMLNetworkServer::disconnectServer() {

    if (connection == NULL) {
        qDebug() << "No connection";
        return false;
    }

    connection->close();
    delete connection;
    connection = NULL;

    return true;
}

void spineMLNetworkServer::readyToRead() {
    qDebug() << "Data available";
}
