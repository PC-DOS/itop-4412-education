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

#include <QApplication>
#include <QHostAddress>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QReadWriteLock>
#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QVector>

/* TCP Networking Data Sending Thread Worker Object */
//This object is moved to a child thread to have its own event loop
class TCPClientDataSender : public QTcpSocket {
    Q_OBJECT

public:
    explicit TCPClientDataSender();
    ~TCPClientDataSender();

    /* Data Sending Status Indicator */
    bool IsDataSending() const;

public slots:
    /* Connection Management Command Handlers */
    void ConnectToServerRequestedEventHandler(const QString sServerIPNew, quint16 iPortNew,
                                              bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew, bool bWairForOperationToComplete);
    void DisconnectFromServerRequestedEventHandler(bool bWairForOperationToComplete);
    void SetAutoReconnectOptionsRequestedEventHandler(bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew);
    void SendDataToServerRequestedEventHandler();
    void StopDataSendingRequestedEventHandler();

signals:
    /* Signals to Communicate with Controller */
    void SocketConnectedToServerEvent(QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void SocketDisconnectedFromServerEvent(QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void SocketErrorOccurredEvent(QAbstractSocket::SocketError errErrorInfo, QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void SocketResponseReceivedFromServerEvent(QString sResponse, QString sServerName, QString sServerIPAddress, quint16 iServerPort);

private:
    QString sServerIP; //INTERNAL: Remote IP Address
    quint16 iPort; //INTERNAL: Remote port
    bool bIsAutoReconnectEnabled; //INTERNAL: Is auto reconnect function on
    unsigned int iAutoReconnectDelay; //INTERNAL: Auto reconnect retry interval
    bool bIsUserInitiatedDisconnection; //INTERNAL: Marks if user has initiated a disconnection, to avoid unexpected TryReconnect() flooding
    bool bIsReconnecting; //INTERNAL: Marks if we are alreading waiting a reconnection, to avoid unexpected TryReconnect() flooding
    volatile bool bIsDataSending; //INTERNAL: Marks if we are sending data, avoid recursive calling of SendDataToServerRequestedEventHandler() and segmentation faults
    volatile bool bIsDataSendingStopRequested; //INTERNAL: Marks if controller has requested to stop data sending

private slots:
    /* TCP Socket Event Handler Slots */
    void TCPClientDataSender_Connected();
    void TCPClientDataSender_Disconnected();
    void TCPClientDataSender_Error(QAbstractSocket::SocketError errErrorInfo);
    void TCPClientDataSender_ReadyRead();

    /* Functional Slots */
    void TryReconnect();
};

/* TCP Networking Client Wrapper */
class TCPClient : public QObject {
    Q_OBJECT

public:
    TCPClient(); //Default constructor, loads options from ini file or default values
    TCPClient(const QString sServerIPNew, quint16 iPortNew,
              bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew); //Constructor with options. Will update options saved in ini file
    ~TCPClient();

    /* Options Management */
    void LoadSettings(); //Load settings from external ini file
    void SaveSettings() const; //Save settings to external ini file

    /* TCP Socket Object Management */
    bool IsConnected() const; //Get if we have connected to a remote server

    /* Connection Management */
    void SetServerParameters(const QString sServerIPNew, quint16 iPortNew); //Host information (IP & Port)
    const QString & GetServerIP() const;
    quint16 GetServerPort() const;
    void ConnectToServer(bool bWairForOperationToComplete = false); //Connect to remote server with saved values
    void ConnectToServer(const QString sServerIPNew, quint16 iPortNew,
                         bool bIsAutoReconnectEnabledNew = false, unsigned int iAutoReconnectDelayNew = 0, bool bWairForOperationToComplete = false); //Connect to remote server with given address and port. Will update options saved in ini file
    void DisconnectFromServer(bool bWairForOperationToComplete = false); //Disconnect
    void SendDataToServer();
    void StopDataSending();
    void PurgeDataFrameQueue(); //Force to purge DataFrameQueue

    /* Data Frame Queue Management */
    void QueueDataFrame(const QString & sData); //Queue a data frame

    /* Options */
    void SetAutoReconnectMode(bool bIsAutoReconnectEnabledNew); //Set & Get auto reconnect function (handles error events)
    bool GetIsAutoReconnectEnabled() const;
    void SetAutoReconnectDelay(unsigned int iAutoReconnectDelayNew); //Set & Get auto reconnect retry interval
    unsigned int GetAutoReconnectDelay() const;

    /* Validators */
    bool IsValidIPAddress(const QString sIPAddress) const; //Check if the given address is valid
    bool IsValidTCPPort(quint16 iPort, bool bUseRegisteredPortsOnly = true) const; //Check if the given port ID is valid (typically in the range of [1,65535], or [1024,32767] if bUseRegisteredPortsOnly is true)

public slots:
    /* Worker Object Event Handler */
    void SocketResponseReceivedFromServerEventHandler(QString sResponse, QString sServerName, QString sServerIPAddress, quint16 iServerPort);

signals:
    /* Signals to Communicate with Worker Object */
    void ConnectToServerRequestedEvent(const QString sServerIPNew, quint16 iPortNew,
                                       bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew, bool bWairForOperationToComplete);
    void DisconnectFromServerRequestedEvent(bool bWairForOperationToComplete);
    void SetAutoReconnectOptionsRequestedEvent(bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew);
    void SendDataToServerRequestedEvent();
    void StopDataSendingRequestedEvent();

    /* Signals to Communicate with Upper Layer */
    void ResponseReceivedFromServerEvent(QString sResponse, QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void ConnectedToServerEvent(QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void DisconnectedFromServerEvent(QString sServerName, QString sServerIPAddress, quint16 iServerPort);
    void NetworkingErrorOccurredEvent(QAbstractSocket::SocketError errErrorInfo, QString sServerName, QString sServerIPAddress, quint16 iServerPort);

private:
    /* Threads & Worker Objects */
    QThread * trdTCPDataSender; //Thread which is used to host and control worker thread
    TCPClientDataSender * tcpDataSender; //Worker object

    /* Options Var */
    QString sServerIP; //INTERNAL: Remote IP Address
    quint16 iPort; //INTERNAL: Remote port
    bool bIsAutoReconnectEnabled; //INTERNAL: Is auto reconnect function on
    unsigned int iAutoReconnectDelay; //INTERNAL: Auto reconnect retry interval
};

/* TCP Client */
extern TCPClient * tcpDataClient;

#endif // NETWORKINGCONTROLINTERFACE_CLIENT_H
