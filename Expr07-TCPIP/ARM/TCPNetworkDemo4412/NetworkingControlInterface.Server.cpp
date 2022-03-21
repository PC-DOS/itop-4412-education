#include "NetworkingControlInterface.Server.h"
#include "SettingsProvider.h"

/* TCP Server */
TCPServer * tcpCommandServer;

/* TCP Server Socket Object */
TCPServerSocket::TCPServerSocket(){
    //Connect events and handlers
    connect(this, SIGNAL(readyRead()), this, SLOT(CommandReceivedFromClientEventHandler()));
    connect(this, SIGNAL(connected()), this, SLOT(TCPServerSocket_Connected()));
    connect(this, SIGNAL(disconnected()), this, SLOT(TCPServerSocket_Disconnected()));
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError"); //Register QAbstractSocket::SocketError type for QueuedConnection
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPServerSocket_Error(QAbstractSocket::SocketError)));
}

TCPServerSocket::~TCPServerSocket(){

}

/* Command Incoming Event Handler Slot */
void TCPServerSocket::CommandReceivedFromClientEventHandler(){
    while (bytesAvailable()){
        //Read a command line and emit a signal
        emit SocketCommandReceivedFromClientEvent(readLine());

        //Process events
        QApplication::processEvents();
    }
    return;
}

/* TCP Socket Event Handler Slots */
void TCPServerSocket::TCPServerSocket_Connected(){
    qDebug()<<"TCPServer: Connected established with remote client"<<peerAddress()<<":"<<peerPort()<<".";
    return;
}

void TCPServerSocket::TCPServerSocket_Disconnected(){
    qDebug()<<"TCPServer: Remote client"<<peerAddress()<<"disconnected.";
    this->deleteLater(); //Delete this object safely
    return;
}

void TCPServerSocket::TCPServerSocket_Error(QAbstractSocket::SocketError errErrorInfo){
    qDebug()<<"TCPServer: Error"<<errErrorInfo<<"occurred, connection aborted.";
    disconnect();
    return;
}

/* TCP Server Object */
TCPServer::TCPServer(){
    //Load settings
    TCPServer::LoadSettings();
}

TCPServer::TCPServer(quint16 iListeningPort){
    //Save settings
    _iListeningPort=iListeningPort;
    TCPServer::SaveSettings();
}

TCPServer::~TCPServer(){
    //Save settings
    TCPServer::SaveSettings();

    //Close server
    TCPServer::StopListening();
}

/* Listening Status Management */
bool TCPServer::StartListening(){
    if (listen(QHostAddress::Any, _iListeningPort)){
        qDebug()<<"TCPServer: Started listening on port"<<_iListeningPort;
        return true;
    }
    else{
        qDebug()<<"TCPServer: Couldnot start listening on port"<<_iListeningPort;
        return false;
    }
    return false;
}

bool TCPServer::StartListening(quint16 iListeningPort){
    //Save settings
    _iListeningPort=iListeningPort;
    TCPServer::SaveSettings();

    //Try starting listening
    if (listen(QHostAddress::Any, _iListeningPort)){
        qDebug()<<"TCPServer: Started listening on port"<<_iListeningPort;
        return true;
    }
    else{
        qDebug()<<"TCPServer: Couldnot start listening on port"<<_iListeningPort;
        return false;
    }
    return false;
}

void TCPServer::StopListening(){
    qDebug()<<"TCPServer: Server closed";
    close(); //Close the sever and stop listening
    return;
}

/* Command Incoming Event Handler Slot */
void TCPServer::SocketCommandReceivedFromClientEventHandler(QString sCommand){
    qDebug()<<"TCPServer: Command"<<sCommand<<"received from the remote";
    emit CommandReceivedEvent(sCommand);
    return;
}

/* Incoming Connection Management */
void TCPServer::incomingConnection(int iSocketID){
    //Create a new socket object
    TCPServerSocket * tcpSocket = new TCPServerSocket;
    tcpSocket->setSocketDescriptor(iSocketID);

    //Connect events and handlers
    connect(tcpSocket, SIGNAL(SocketCommandReceivedEvent(QString)), this, SLOT(SocketCommandReceivedFromClientEventHandler(QString)));

    return;
}

/* Options Management */
void TCPServer::LoadSettings(){
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    _iListeningPort=SettingsContainer.value(ST_KEY_LISTENING_PORT,ST_DEFVAL_LISTENING_PORT).toUInt();
    SettingsContainer.endGroup();
    return;
}

void TCPServer::SaveSettings() const{
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    SettingsContainer.setValue(ST_KEY_LISTENING_PORT,_iListeningPort);
    SettingsContainer.endGroup();
    return;
}
