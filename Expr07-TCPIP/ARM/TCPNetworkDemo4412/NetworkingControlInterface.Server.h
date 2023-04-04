/*
 * NETWORKING CONTROL INTERFACE :: SERVER
 *
 * This file is the interface of networking interface (server side).
 * Working as a TCP server, receive commands (mostly line-based string commands) from the remote and then parse & execute them.
 * NOTE: This interface should only be used to receive commands from remote controller. If you want to implement an interface that receives wave data from the remote device, please consider implementing a new DeviceControlInterface.
 *
 * This file is a part of DataSourceProvider, but was separated for easier maintainance.
 * For DataFrames' definitions and stream operators, please refer to DataSourceProvider.
 *
 */
#ifndef NETWORKINGCONTROLINTERFACE_SERVER_H
#define NETWORKINGCONTROLINTERFACE_SERVER_H

#include <QApplication>
#include <QHostAddress>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QReadWriteLock>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QVector>

/* TCP Server Socket Object */
//This object maintains a connection from a local TCP server to a remote TCP client
class TCPServerSocket : public QTcpSocket {
    Q_OBJECT

public:
    TCPServerSocket();
    ~TCPServerSocket();

public slots:
    /* Text-Based Communication */
    void SendDataToClientRequestedEventHandler(QString sDataToSend); //Send data to client

signals:
    void SocketCommandReceivedFromClientEvent(QString sCommand, QString sClientIP, quint16 iClientPort); //Signal that informs the TCP Server Object a command received from the remote client

private slots:
    /* Command Incoming Event Handler Slot */
    void CommandReceivedFromClientEventHandler(); //Process incoming commands

    /* TCP Socket Event Handler Slots */
    void TCPServerSocket_Connected();
    void TCPServerSocket_Disconnected();
    void TCPServerSocket_Error(QAbstractSocket::SocketError errErrorInfo);
};

/* TCP Server Object */
class TCPServer : public QTcpServer {
    Q_OBJECT

public:
    TCPServer();
    TCPServer(quint16 iListeningPortInit); //Construct the object with a given listening port
    ~TCPServer();

    /* Options Management */
    void LoadSettings(); //INTERNAL: Load settings from external ini file
    void SaveSettings() const; //INTERNAL: Save settings to external ini file

    /* Listening Status Management */
    bool StartListening(); //Start listening on saved port
    bool StartListening(quint16 iListeningPortNew); //Start listening on a given port
    void StopListening(); //Stop listening

    /* Text-Based Communication */
    void SendDataToClient(QString sDataToSend);

signals:
    /* Signals to Communicate with Upper Layer */
    void ClientConnectedEvent(QString sClientIPAddress, quint16 iClientPort); //Signal of a connected client
    void CommandReceivedEvent(QString sCommand, QString sClientIP, quint16 iClientPort); //Signal of a received command

    /* Signals to Communicate with Client */
    void SendDataToClientRequestedEvent(QString sDataToSend); //Signal of requesting sending data to client

private slots:
    /* Command Incoming Event Handler Slot */
    void SocketCommandReceivedFromClientEventHandler(QString sCommand, QString sClientIP, quint16 iClientPort); //Receive a command from a socket, and then post a new event to infrom upper layers

private:
    /* Options Var */
    quint16 iListeningPort; //INTERNAL: Listening port

    /* Incoming Connection Management */
    void incomingConnection(int iSocketID); //Reimplement incomingConnecting() function, create a new socket object
};

/* TCP Server */
extern TCPServer * tcpCommandServer;

#endif // NETWORKINGCONTROLINTERFACE_SERVER_H
