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
    //Create TCP socket object and connect events
    connect(this, SIGNAL(connected()), this, SLOT(TCPClientDataSender_Connected()), Qt::QueuedConnection);
    connect(this, SIGNAL(disconnected()), this, SLOT(TCPClientDataSender_Disconnected()), Qt::QueuedConnection);
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError"); //Register QAbstractSocket::SocketError type for QueuedConnection
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPClientDataSender_Error(QAbstractSocket::SocketError)), Qt::QueuedConnection);
    connect(this, SIGNAL(readyRead()), this, SLOT(TCPClientDataSender_ReadyRead()), Qt::QueuedConnection);
}

TCPClientDataSender::~TCPClientDataSender(){
}

/* Connection Management Command Handlers */
void TCPClientDataSender::ConnectToServerEventHandler(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay, bool WairForOperationToComplete){
    _sServerIP=sServerIP;
    _iPort=iPort;
    _IsAutoReconnectEnabled=IsAutoReconnectEnabled;
    _iAutoReconnectDelay=iAutoReconnectDelay;

    connectToHost(sServerIP, iPort);
    if (WairForOperationToComplete){
        waitForConnected();
    }

    return;
}

void TCPClientDataSender::DisconnectFromServerEventHandler(bool WairForOperationToComplete){
    disconnectFromHost();
    if (WairForOperationToComplete){
        waitForDisconnected();
    }
    return;
}

void TCPClientDataSender::SetAutoReconnectOptionsEventHandler(bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay){
    _IsAutoReconnectEnabled=IsAutoReconnectEnabled;
    _iAutoReconnectDelay=iAutoReconnectDelay;
    return;
}

void TCPClientDataSender::SendDataToServerEventHandler(){
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
    return;
}

/* TCP Socket Event Handler Slots */
void TCPClientDataSender::TCPClientDataSender_Connected(){
    qDebug()<<"TCPClient: Connected to"<<_sServerIP<<":"<<_iPort;
    return;
}

void TCPClientDataSender::TCPClientDataSender_Disconnected(){
    qDebug()<<"TCPClient: Disconnected from"<<_sServerIP<<":"<<_iPort;
    return;
}

void TCPClientDataSender::TCPClientDataSender_Error(QAbstractSocket::SocketError errErrorInfo){
    qDebug()<<"TCPClient: Error:"<<errErrorInfo;
    disconnectFromHost();
    for (int i = 0; i <= INT_MAX; ++i){ //Manually implies WaitForDisonnected;
        if (state()==QTcpSocket::UnconnectedState){ //Check if we have disconnected
            break;
        }
        QApplication::processEvents(); //Call QApplication::processEvents() to process event loop (Avoid jamming main thread)
    }
    if (_IsAutoReconnectEnabled){ //Check if we need to reconnect
        qDebug()<<"TCPClient: Will retry connect after"<<_iAutoReconnectDelay<<"ms";
        QTimer::singleShot(_iAutoReconnectDelay, this, SLOT(TryReconnect()));
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
    qDebug()<<"TCPClient: Retrying to connect to"<<_sServerIP<<":"<<_iPort;
    connectToHost(_sServerIP, _iPort);
    for (int i = 0; i <= INT_MAX; ++i){ //Manually implies WaitForConnected;
        if (state()==QTcpSocket::ConnectedState){ //Check if we have connected
            qDebug()<<"TCPClient: Reconnected to"<<_sServerIP<<":"<<_iPort;
            break;
        }
        QApplication::processEvents(); //Call QApplication::processEvents() to process event loop (Avoid jamming main thread)
    }
    if (state()!=QTcpSocket::ConnectedState){
        qDebug()<<"TCPClient: Will retry connect after"<<_iAutoReconnectDelay<<"ms";
        disconnectFromHost();
        for (int i = 0; i <= INT_MAX; ++i){ //Manually implies WaitForDisonnected;
            if (state()==QTcpSocket::UnconnectedState){ //Check if we have disconnected
                break;
            }
            QApplication::processEvents(); //Call QApplication::processEvents() to process event loop (Avoid jamming main thread)
        }
        QTimer::singleShot(_iAutoReconnectDelay, this, SLOT(TryReconnect()));
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
    connect(this, SIGNAL(ConnectToServerEvent(QString,quint16,bool,unsigned int,bool)), tcpDataSender, SLOT(ConnectToServerEventHandler(QString,quint16,bool,unsigned int,bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(DisconnectFromServerEvent(bool)), tcpDataSender, SLOT(DisconnectFromServerEventHandler(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(SetAutoReconnectOptionsEvent(bool,uint)), tcpDataSender, SLOT(SetAutoReconnectOptionsEventHandler(bool,uint)), Qt::QueuedConnection);
    connect(this, SIGNAL(SendDataToServerEvent()), tcpDataSender, SLOT(SendDataToServerEventHandler()), Qt::QueuedConnection);
    connect(tcpDataSender, SIGNAL(SocketResponseReceivedFromServerEvent(QString)), this, SLOT(SocketResponseReceivedFromServerEventHandler(QString)), Qt::QueuedConnection);

    //Start child thread's own event loop
    trdTCPDataSender->start();
}

TCPClient::TCPClient(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay){
    //Save settings
    _sServerIP=sServerIP;
    _iPort=iPort;
    _IsAutoReconnectEnabled=IsAutoReconnectEnabled;
    _iAutoReconnectDelay=iAutoReconnectDelay;
    TCPClient::SaveSettings();

    //Create worker object
    tcpDataSender = new TCPClientDataSender;

    //Creat thread object
    trdTCPDataSender = new QThread;
    tcpDataSender->moveToThread(trdTCPDataSender);

    //Connect events and handlers
    connect(this,SIGNAL(ConnectToServerEvent(QString,quint16,bool,unsigned int,bool)),tcpDataSender,SLOT(ConnectToServerEventHandler(QString,quint16,bool,unsigned int,bool)),Qt::QueuedConnection);
    connect(this,SIGNAL(DisconnectFromServerEvent(bool)),tcpDataSender,SLOT(DisconnectFromServerEventHandler(bool)),Qt::QueuedConnection);
    connect(this,SIGNAL(SetAutoReconnectOptionsEvent(bool,uint)),tcpDataSender,SLOT(SetAutoReconnectOptionsEventHandler(bool,uint)),Qt::QueuedConnection);
    connect(this,SIGNAL(SendDataToServerEvent()),tcpDataSender,SLOT(SendDataToServerEventHandler()),Qt::QueuedConnection);
    connect(tcpDataSender, SIGNAL(SocketResponseReceivedFromServerEvent(QString)), this, SLOT(SocketResponseReceivedFromServerEventHandler(QString)), Qt::QueuedConnection);

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
void TCPClient::SetServerParameters(const QString sServerIP, quint16 iPort){
    if (TCPClient::IsValidIPAddress(sServerIP) && TCPClient::IsValidTCPPort(iPort)){
        //Save settings
        _sServerIP=sServerIP;
        _iPort=iPort;
        TCPClient::SaveSettings();
    }
    return;
}

const QString & TCPClient::GetServerIP() const{
    return _sServerIP;
}

quint16 TCPClient::GetServerPort() const{
    return _iPort;
}

void TCPClient::ConnectToServer(bool WairForOperationToComplete){
    emit ConnectToServerEvent(_sServerIP,_iPort,_IsAutoReconnectEnabled,_iAutoReconnectDelay,WairForOperationToComplete);
    return;
}

void TCPClient::ConnectToServer(const QString sServerIP, quint16 iPort, bool IsAutoReconnectEnabled, unsigned int iAutoReconnectDelay, bool WairForOperationToComplete){
    //Save settings
    _sServerIP=sServerIP;
    _iPort=iPort;
    _IsAutoReconnectEnabled=IsAutoReconnectEnabled;
    _iAutoReconnectDelay=iAutoReconnectDelay;
    TCPClient::SaveSettings();

    //Connect
    emit ConnectToServerEvent(_sServerIP,_iPort,_IsAutoReconnectEnabled,_iAutoReconnectDelay,WairForOperationToComplete);
    return;
}

void TCPClient::DisconnectFromServer(bool WairForOperationToComplete){
    emit DisconnectFromServerEvent(WairForOperationToComplete);
    return;
}

void TCPClient::SendDataToServer(){
    emit SendDataToServerEvent();
    return;
}

/* Data Frame Queue Management */
void TCPClient::QueueDataFrame(const QString &sData){
    mtxDataFramesPendingSendingLock.lock(); //Begin writing internal buffer

    if (queDataFramesPendingSending.size()>NET_DATA_QUEUE_MAX_ITEM_COUNT){
        queDataFramesPendingSending.clear();
        qDebug()<<"TCPClient: Data queue has been purged because it has exceeded the size limit.";
    }

    queDataFramesPendingSending.enqueue(new QString(sData));

    mtxDataFramesPendingSendingLock.unlock(); //Don't forget to unlock me!

    return;
}

/* Options */
void TCPClient::SetAutoReconnectMode(bool IsAutoReconnectEnabled){
    _IsAutoReconnectEnabled=IsAutoReconnectEnabled;
    TCPClient::SaveSettings();
    emit SetAutoReconnectOptionsEvent(_IsAutoReconnectEnabled,_iAutoReconnectDelay);
    return;
}

bool TCPClient::GetIsAutoReconnectEnabled() const{
    return _IsAutoReconnectEnabled;
}

void TCPClient::SetAutoReconnectDelay(unsigned int iAutoReconnectDelay){
    _iAutoReconnectDelay=iAutoReconnectDelay;
    TCPClient::SaveSettings();
    emit SetAutoReconnectOptionsEvent(_IsAutoReconnectEnabled,_iAutoReconnectDelay);
    return;
}

unsigned int TCPClient::GetAutoReconnectDelay() const{
    return _iAutoReconnectDelay;
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
    _sServerIP=SettingsContainer.value(ST_KEY_SERVER_IP,ST_DEFVAL_SERVER_IP).toString();
    _iPort=SettingsContainer.value(ST_KEY_SERVER_PORT,ST_DEFVAL_SERVER_PORT).toUInt();
    _IsAutoReconnectEnabled=SettingsContainer.value(ST_KEY_IS_AUTORECONN_ON,ST_DEFVAL_IS_AUTORECONN_ON).toBool();
    _iAutoReconnectDelay=SettingsContainer.value(ST_KEY_AUTORECONN_DELAY_MS,ST_DEFVAL_AUTORECONN_DELAY_MS).toUInt();
    SettingsContainer.endGroup();
    return;
}

void TCPClient::SaveSettings() const{
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    SettingsContainer.setValue(ST_KEY_SERVER_IP, _sServerIP);
    SettingsContainer.setValue(ST_KEY_SERVER_PORT, _iPort);
    SettingsContainer.setValue(ST_KEY_IS_AUTORECONN_ON, _IsAutoReconnectEnabled);
    SettingsContainer.setValue(ST_KEY_AUTORECONN_DELAY_MS, _iAutoReconnectDelay);
    SettingsContainer.sync();
    SettingsContainer.endGroup();
    return;
}
