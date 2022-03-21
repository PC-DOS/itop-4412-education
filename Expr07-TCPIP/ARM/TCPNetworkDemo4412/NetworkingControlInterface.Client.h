/*
 * NETWORKING CONTROL INTERFACE :: CLIENT
 *
 * This file is the interface of networking interface (client side).
 * Working as a TCP client, and transfers data to the remote.
 *
 * This file is a part of DataSourceProvider, but was separated for easier maintainance.
 * For DataFrames' definitions and stream operators, please refer to DataSourceProvider.
 *
 */

#ifndef NETWORKINGCONTROLINTERFACE_CLIENT_H
#define NETWORKINGCONTROLINTERFACE_CLIENT_H

#include <QVector>
#include <QQueue>
#include <QString>
#include <QMap>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>
#include <QTimer>
#include <QApplication>
#include <QHostAddress>

/* TCP Networking Data Sending Thread Worker Object */
//This object is moved to a child thread to have its own event loop
class TCPClientDataSender : public QTcpSocket{
    Q_OBJECT

public:
    explicit TCPClientDataSender();
    ~TCPClientDataSender();

public slots:
    /* Connection Management Command Handlers */
    void ConnectToServerEventHandler(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay, bool WairForOperationToComplete);
    void DisconnectFromServerEventHandler(bool WairForOperationToComplete);
    void SetAutoReconnectOptionsEventHandler(bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay);
    void SendDataToServerEventHandler();

    /* TCP Socket Event Handler Slots */
    void TCPClientDataSender_Connected();
    void TCPClientDataSender_Disconnected();
    void TCPClientDataSender_Error(QAbstractSocket::SocketError errErrorInfo);
    void TCPClientDataSender_ReadyRead();

    /* Functional Slots */
    void TryReconnect();

signals:
    void SocketResponseReceivedFromServerEvent(QString sResponse);

private:
    QString _sServerIP; //INTERNAL: Remote IP Address
    quint16 _iPort; //INTERNAL: Remote port
    bool _IsAutoReconnectEnabled; //INTERNAL: Is auto reconnect function on
    unsigned int _iAutoReconnectDelay; //INTERNAL: Auto reconnect retry interval
};

/* TCP Networking Client Wrapper */
class TCPClient : public QObject{
    Q_OBJECT

public:
    TCPClient(); //Default constructor, loads options from ini file or default values
    TCPClient(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay); //Constructor with options. Will update options saved in ini file
    ~TCPClient();

    /* TCP Socket Object Management */
    bool IsConnected() const; //Get if we have connected to a remote server

    /* Connection Management */
    void SetServerParameters(const QString sServerIP, quint16 iPort); //Host information (IP & Port)
    const QString & GetServerIP() const;
    quint16 GetServerPort() const;
    void ConnectToServer(bool WairForOperationToComplete = false); //Connect to remote server with saved values
    void ConnectToServer(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled=false, unsigned int iAutoReconnectDelay=0, bool WairForOperationToComplete = false); //Connect to remote server with given address and port. Will update options saved in ini file
    void DisconnectFromServer(bool WairForOperationToComplete = false); //Disconnect
    void SendDataToServer();

    /* Data Frame Queue Management */
    void QueueDataFrame(const QString & sData); //Queue a data frame

    /* Options */
    void SetAutoReconnectMode(bool IsAutoReconnectEnabled); //Set & Get auto reconnect function (handles error events)
    bool GetIsAutoReconnectEnabled() const;
    void SetAutoReconnectDelay(unsigned int iAutoReconnectDelay); //Set & Get auto reconnect retry interval
    unsigned int GetAutoReconnectDelay() const;

    /* Validators */
    bool IsValidIPAddress(const QString sIPAddress) const; //Check if the given address is valid
    bool IsValidTCPPort(quint16 iPort, bool UseRegisteredPortsOnly=true) const; //Check if the given port ID is valid (typically in the range of [1,65535], or [1024,32767] if RegisteredPortsOnly is true)

    /* Threads & Worker Objects */
    QThread * trdTCPDataSender; //Thread which is used to host and control worker thread
    TCPClientDataSender * tcpDataSender; //Worker object

public slots:
    /* Worker Object Event Handler */
    void SocketResponseReceivedFromServerEventHandler(QString sResponse);

signals:
    /* Signals to Communicate with Worker Object */
    void ConnectToServerEvent(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay, bool WairForOperationToComplete);
    void DisconnectFromServerEvent(bool WairForOperationToComplete);
    void SetAutoReconnectOptionsEvent(bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay);
    void SendDataToServerEvent();

    /* Signals to Communicate with Upper Layer */
    void ResponseReceivedFromServerEvent(QString sResponse);

private:
    /* Connection Management */
    //volatile bool _IsConnected; //INTERNAL: Get if we have connected to remote server. Not used, use tcpDataSnder->tcpSocket->state() instead.

    /* Options Var */
    QString _sServerIP; //INTERNAL: Remote IP Address
    quint16 _iPort; //INTERNAL: Remote port
    bool _IsAutoReconnectEnabled; //INTERNAL: Is auto reconnect function on
    unsigned int _iAutoReconnectDelay; //INTERNAL: Auto reconnect retry interval

    /* Options Management */
    void LoadSettings(); //INTERNAL: Load settings from external ini file
    void SaveSettings() const; //INTERNAL: Save settings to external ini file
};

/* TCP Client */
extern TCPClient * tcpDataClient;

#endif // NETWORKINGCONTROLINTERFACE_CLIENT_H
