#include "NetworkingControlInterface.Client.h"
#include "SettingsProvider.h"

/* Data Queue */
#define NET_DATA_QUEUE_MAX_ITEM_COUNT 40960 //Max size of data buffer, to avoid huge memory consumption
static QQueue<QString * > queDataFramesPendingSending; //Queue of data frames pending sending

/* Intenral Variables */

/* Internal Locks */
static QMutex mtxDataFramesPendingSendingLock; //For internal buffers, Initialize the lock recursivly

/* TCP Client */
TCPClient * tcpDataClient;

/* TCP Networking Data Sending Thread Worker Object */
TCPClientDataSender::TCPClientDataSender(){
    //Initialize internal variables
    bIsDataSending=false;
    bIsUserInitiatedDisconnection=false;
    bIsReconnecting=false;

    //Create TCP socket object and connect events
    connect(this, SIGNAL(connected()), this, SLOT(TCPClientDataSender_Connected()));
    connect(this, SIGNAL(disconnected()), this, SLOT(TCPClientDataSender_Disconnected()));
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError"); //Register QAbstractSocket::SocketError type for QueuedConnection
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPClientDataSender_Error(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(readyRead()), this, SLOT(TCPClientDataSender_ReadyRead()));
}

TCPClientDataSender::~TCPClientDataSender(){
}

/* Data Sending Status Indicator */
bool TCPClientDataSender::IsDataSending() const{
    return bIsDataSending;
}

/* Connection Management Command Handlers */
void TCPClientDataSender::ConnectToServerEventHandler(const QString sServerIPNew, quint16 iPortNew,
                                                      bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew, bool bWairForOperationToCompleteNew){
    sServerIP=sServerIPNew;
    iPort=iPortNew;
    bIsAutoReconnectEnabled=bIsAutoReconnectEnabledNew;
    iAutoReconnectDelay=iAutoReconnectDelayNew;
    bIsUserInitiatedDisconnection=false;

    connectToHost(sServerIPNew, iPortNew);
    if (bWairForOperationToCompleteNew){
        waitForConnected();
    }

    return;
}

void TCPClientDataSender::DisconnectFromServerEventHandler(bool bWairForOperationToComplete){
    disconnectFromHost();
    bIsUserInitiatedDisconnection=true;
    bIsReconnecting=false;
    if (bWairForOperationToComplete){
        waitForDisconnected();
    }
    return;
}

void TCPClientDataSender::SetAutoReconnectOptionsEventHandler(bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew){
    bIsAutoReconnectEnabled=bIsAutoReconnectEnabledNew;
    iAutoReconnectDelay=iAutoReconnectDelayNew;
    return;
}

void TCPClientDataSender::SendDataToServerEventHandler(){
    //Check if SendDataToServerEventHandler() is running, avoid recursive calling of SendDataToServerEventHandler() and segmentation faults
    if (bIsDataSending){
        //If this is a recursive calling, simply returns
        return;
    }
    else{
        //Marks SendDataToServerEventHandler() is running
        bIsDataSending=true;
    }

    while (!queDataFramesPendingSending.empty() && state()==QTcpSocket::ConnectedState){ //Sends all queued data frames to remote
        QString * frmCurrentSendingDataFrame = NULL;

        if (mtxDataFramesPendingSendingLock.tryLock()){ //Begin reading internal buffer
            frmCurrentSendingDataFrame=queDataFramesPendingSending.dequeue();
            mtxDataFramesPendingSendingLock.unlock(); //Don't forget to unlock me!
        }
        else{
            break; //If we can't lock internal buffer, just stop this try
        }

        if (!frmCurrentSendingDataFrame){ //Check if we got null
            continue;
        }

        //Send data
        write(frmCurrentSendingDataFrame->toLatin1()); //Convert QString to ASCII sequence
        flush();

        //Free memory space
        delete frmCurrentSendingDataFrame;
        frmCurrentSendingDataFrame=NULL;

        //Process all events
        QApplication::processEvents();
    }
    
    bIsDataSending=false;
    return;
}

/* TCP Socket Event Handler Slots */
void TCPClientDataSender::TCPClientDataSender_Connected(){
    qDebug()<<"TCPClient: Connected to"<<sServerIP<<":"<<iPort;
    bIsReconnecting=false;
    return;
}

void TCPClientDataSender::TCPClientDataSender_Disconnected(){
    qDebug()<<"TCPClient: Disconnected from"<<sServerIP<<":"<<iPort;
    return;
}

void TCPClientDataSender::TCPClientDataSender_Error(QAbstractSocket::SocketError errErrorInfo){
    qDebug()<<"TCPClient: Error"<<errErrorInfo<<": "<<errorString();
    if (state() != QTcpSocket::UnconnectedState){
        disconnectFromHost();
        waitForDisconnected(245000);
    }
    if (bIsAutoReconnectEnabled && !bIsReconnecting && !bIsUserInitiatedDisconnection){ //Check if we need to reconnect
        qDebug()<<"TCPClient: Will retry connect after"<<iAutoReconnectDelay<<"ms";
        QTimer::singleShot(iAutoReconnectDelay, this, SLOT(TryReconnect()));
        bIsReconnecting=true;
    }
    return;
}

void TCPClientDataSender::TCPClientDataSender_ReadyRead(){
    while (bytesAvailable()){
        //Read a command line and emit a signal
        emit SocketResponseReceivedFromServerEvent(readLine());

        //Process events
        QApplication::processEvents();
    }
    return;
}

/* Functional Slots */
void TCPClientDataSender::TryReconnect(){
    if (bIsUserInitiatedDisconnection){
        return;
    }
    if (state() != QTcpSocket::ConnectingState && state() != QTcpSocket::ConnectedState){
        qDebug()<<"TCPClient: Retrying to connect to"<<sServerIP<<":"<<iPort;
        connectToHost(sServerIP, iPort);
        waitForConnected(245000);
    }
    if (state() != QTcpSocket::ConnectedState){
        qDebug()<<"TCPClient: Will retry connect after"<<iAutoReconnectDelay<<"ms";
        if (state() != QTcpSocket::UnconnectedState){
            disconnectFromHost();
            waitForDisconnected(245000);
        }
        QTimer::singleShot(iAutoReconnectDelay, this, SLOT(TryReconnect()));
    }
    return;
}

/* TCP Networking Client Wrapper */
TCPClient::TCPClient(){
    //Load settings
    TCPClient::LoadSettings();

    //Create worker object
    tcpDataSender = new TCPClientDataSender;

    //Creat thread object and move worker object (and all it's child objects) to this thread
    trdTCPDataSender = new QThread;
    tcpDataSender->moveToThread(trdTCPDataSender);

    //Connect events and handlers
    connect(this, SIGNAL(ConnectToServerEvent(QString,quint16,bool,unsigned int,bool)), tcpDataSender, SLOT(ConnectToServerEventHandler(QString,quint16,bool,unsigned int,bool)));
    connect(this, SIGNAL(DisconnectFromServerEvent(bool)), tcpDataSender, SLOT(DisconnectFromServerEventHandler(bool)));
    connect(this, SIGNAL(SetAutoReconnectOptionsEvent(bool,uint)), tcpDataSender, SLOT(SetAutoReconnectOptionsEventHandler(bool,uint)));
    connect(this, SIGNAL(SendDataToServerEvent()), tcpDataSender, SLOT(SendDataToServerEventHandler()));
    connect(tcpDataSender, SIGNAL(SocketResponseReceivedFromServerEvent(QString)), this, SLOT(SocketResponseReceivedFromServerEventHandler(QString)));
    connect(tcpDataSender, SIGNAL(connected()), this, SIGNAL(ConnectedToServerEvent()));
    connect(tcpDataSender, SIGNAL(disconnected()), this, SIGNAL(DisconnectedFromServerEvent()));
    connect(tcpDataSender, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(NetworkingErrorEvent(QAbstractSocket::SocketError)));

    //Start child thread's own event loop
    trdTCPDataSender->start();
}

TCPClient::TCPClient(const QString sServerIPNew, quint16 iPortNew,
                     bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew){
    //Save settings
    sServerIP=sServerIPNew;
    iPort=iPortNew;
    bIsAutoReconnectEnabled=IsAutoReconnectEnabledNew;
    iAutoReconnectDelay=iAutoReconnectDelayNew;
    TCPClient::SaveSettings();

    //Create worker object
    tcpDataSender = new TCPClientDataSender;

    //Creat thread object
    trdTCPDataSender = new QThread;
    tcpDataSender->moveToThread(trdTCPDataSender);

    //Connect events and handlers
    connect(this, SIGNAL(ConnectToServerEvent(QString,quint16,bool,unsigned int,bool)), tcpDataSender, SLOT(ConnectToServerEventHandler(QString,quint16,bool,unsigned int,bool)));
    connect(this, SIGNAL(DisconnectFromServerEvent(bool)), tcpDataSender, SLOT(DisconnectFromServerEventHandler(bool)));
    connect(this, SIGNAL(SetAutoReconnectOptionsEvent(bool,uint)), tcpDataSender, SLOT(SetAutoReconnectOptionsEventHandler(bool,uint)));
    connect(this, SIGNAL(SendDataToServerEvent()), tcpDataSender, SLOT(SendDataToServerEventHandler()));
    connect(tcpDataSender, SIGNAL(SocketResponseReceivedFromServerEvent(QString)), this, SLOT(SocketResponseReceivedFromServerEventHandler(QString)));
    connect(tcpDataSender, SIGNAL(connected()), this, SIGNAL(ConnectedToServerEvent()));
    connect(tcpDataSender, SIGNAL(disconnected()), this, SIGNAL(DisconnectedFromServerEvent()));
    connect(tcpDataSender, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(NetworkingErrorEvent(QAbstractSocket::SocketError)));

    //Start child thread's own event loop
    trdTCPDataSender->start();
}

TCPClient::~TCPClient(){
    //Save settings
    if (TCPClient::IsConnected()){
        TCPClient::DisconnectFromServer();
    }
    TCPClient::SaveSettings();

    //Quit child thread
    trdTCPDataSender->quit();
    if (!trdTCPDataSender->wait(1000)){
        trdTCPDataSender->terminate();
    }

    //Delete worker object
    tcpDataSender->deleteLater();
    tcpDataSender=NULL;
    
    //Delete child thread
    trdTCPDataSender->deleteLater();
    trdTCPDataSender=NULL;
}

/* TCP Socket Object Management */
bool TCPClient::IsConnected() const{
    return (tcpDataSender->state()==QTcpSocket::ConnectedState);
}

/* Connection Management */
void TCPClient::SetServerParameters(const QString sServerIPNew, quint16 iPortNew){
    if (TCPClient::IsValidIPAddress(sServerIPNew) && TCPClient::IsValidTCPPort(iPortNew)){
        //Save settings
        sServerIP=sServerIPNew;
        iPort=iPortNew;
        TCPClient::SaveSettings();
    }
    return;
}

const QString & TCPClient::GetServerIP() const{
    return sServerIP;
}

quint16 TCPClient::GetServerPort() const{
    return iPort;
}

void TCPClient::ConnectToServer(bool bWairForOperationToComplete){
    emit ConnectToServerEvent(sServerIP,iPort,bIsAutoReconnectEnabled,iAutoReconnectDelay,bWairForOperationToComplete);
    return;
}

void TCPClient::ConnectToServer(const QString sServerIPNew, quint16 iPortNew,
                                bool bIsAutoReconnectEnabledNew, unsigned int iAutoReconnectDelayNew, bool bWairForOperationToComplete){
    //Save settings
    sServerIP=sServerIPNew;
    iPort=iPortNew;
    bIsAutoReconnectEnabled=bIsAutoReconnectEnabledNew;
    iAutoReconnectDelay=iAutoReconnectDelayNew;
    TCPClient::SaveSettings();

    //Connect
    emit ConnectToServerEvent(sServerIP,iPort,bIsAutoReconnectEnabled,iAutoReconnectDelay,bWairForOperationToComplete);
    return;
}

void TCPClient::DisconnectFromServer(bool bWairForOperationToComplete){
    emit DisconnectFromServerEvent(bWairForOperationToComplete);
    return;
}

void TCPClient::SendDataToServer(){
    //In high frame rate mode, avoid SendDataToServerRequestedEvent() signal flooding
    if (tcpDataSender->IsDataSending()){
        return;
    }
    emit SendDataToServerEvent();
    return;
}

/* Data Frame Queue Management */
void TCPClient::QueueDataFrame(const QString &sData){
    mtxDataFramesPendingSendingLock.lock(); //Begin writing internal buffer

    if (queDataFramesPendingSending.size()>NET_DATA_QUEUE_MAX_ITEM_COUNT){
        while (!queDataFramesPendingSending.empty()){
            delete queDataFramesPendingSending.dequeue();
        }
        qDebug()<<"TCPClient: Data queue has been purged because it has exceeded the size limit.";
    }

    queDataFramesPendingSending.enqueue(new QString(sData));

    mtxDataFramesPendingSendingLock.unlock(); //Don't forget to unlock me!

    return;
}

/* Options */
void TCPClient::SetAutoReconnectMode(bool bIsAutoReconnectEnabledNew){
    bIsAutoReconnectEnabled=bIsAutoReconnectEnabledNew;
    TCPClient::SaveSettings();
    emit SetAutoReconnectOptionsEvent(bIsAutoReconnectEnabled,iAutoReconnectDelay);
    return;
}

bool TCPClient::GetIsAutoReconnectEnabled() const{
    return bIsAutoReconnectEnabled;
}

void TCPClient::SetAutoReconnectDelay(unsigned int iAutoReconnectDelayNew){
    iAutoReconnectDelay=iAutoReconnectDelayNew;
    TCPClient::SaveSettings();
    emit SetAutoReconnectOptionsEvent(bIsAutoReconnectEnabled,iAutoReconnectDelay);
    return;
}

unsigned int TCPClient::GetAutoReconnectDelay() const{
    return iAutoReconnectDelay;
}

/* Worker Object Event Handler */
void TCPClient::SocketResponseReceivedFromServerEventHandler(QString sResponse){
    qDebug()<<"TCPClient: Response"<<sResponse<<"received from the remote";
    emit ResponseReceivedFromServerEvent(sResponse);
    return;
}

/* Validators */
bool TCPClient::IsValidIPAddress(const QString sIPAddress) const{
    QHostAddress hstTestAddr;
    return hstTestAddr.setAddress(sIPAddress);
}

bool TCPClient::IsValidTCPPort(quint16 iPort, bool UseRegisteredPortsOnly) const{
    quint16 iPortIDMin = 1, iPortIDMax = 65535;
    if (UseRegisteredPortsOnly){
        iPortIDMin=1024;
        iPortIDMax=32767;
    }
    return ((iPort >= iPortIDMin) && (iPort <= iPortIDMax));
}

/* Options Management */
void TCPClient::LoadSettings(){
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    sServerIP=SettingsContainer.value(ST_KEY_SERVER_IP,ST_DEFVAL_SERVER_IP).toString();
    iPort=SettingsContainer.value(ST_KEY_SERVER_PORT,ST_DEFVAL_SERVER_PORT).toUInt();
    bIsAutoReconnectEnabled=SettingsContainer.value(ST_KEY_IS_AUTORECONN_ON,ST_DEFVAL_IS_AUTORECONN_ON).toBool();
    iAutoReconnectDelay=SettingsContainer.value(ST_KEY_AUTORECONN_DELAY_MS,ST_DEFVAL_AUTORECONN_DELAY_MS).toUInt();
    SettingsContainer.endGroup();
    return;
}

void TCPClient::SaveSettings() const{
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    SettingsContainer.setValue(ST_KEY_SERVER_IP, sServerIP);
    SettingsContainer.setValue(ST_KEY_SERVER_PORT, iPort);
    SettingsContainer.setValue(ST_KEY_IS_AUTORECONN_ON, bIsAutoReconnectEnabled);
    SettingsContainer.setValue(ST_KEY_AUTORECONN_DELAY_MS, iAutoReconnectDelay);
    SettingsContainer.sync();
    SettingsContainer.endGroup();
    return;
}
