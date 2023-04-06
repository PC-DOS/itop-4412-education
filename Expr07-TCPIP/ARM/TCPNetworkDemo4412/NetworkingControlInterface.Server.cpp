#include "NetworkingControlInterface.Server.h"
#include "SettingsProvider.h"

/* TCP Server */
TCPServer * tcpCommandServer;

/* TCP Server Socket Object */
TCPServerSocket::TCPServerSocket() {
    //Connect events and handlers
    connect(this, SIGNAL(readyRead()), this, SLOT(CommandReceivedFromClientEventHandler()));
    connect(this, SIGNAL(connected()), this, SLOT(TCPServerSocket_Connected()));
    connect(this, SIGNAL(disconnected()), this, SLOT(TCPServerSocket_Disconnected()));
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError"); //Register QAbstractSocket::SocketError type for QueuedConnection
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPServerSocket_Error(QAbstractSocket::SocketError)));
}

TCPServerSocket::~TCPServerSocket() {
}

/* Text-Based Communication */
void TCPServerSocket::SendDataToClientRequestedEventHandler(QString sDataToSend, QString sClientName, QString sClientIP, quint16 iClientPort) {
    /*
    //Add line separator, using Windows mode ("\r\n")
    if (!sDataToSend.endsWith("\r\n")){
        if (sDataToSend.endsWith('\r')){
            sDataToSend+='\n';
        }
        else if (sDataToSend.endsWith('\n')){
            sDataToSend.insert(sDataToSend.length()-1, '\r');
        }
        else{
            sDataToSend+="\r\n";
        }
    }
    */
    //Judge if we need to handle the request
    if ((sClientName == "" && sClientIP == "" && iClientPort == 0) ||
        (sClientName == peerName() && sClientIP == "" && iClientPort == 0) ||
        (sClientName == "" && sClientIP == peerAddress().toString() && iClientPort == 0) ||
        (sClientName == "" && sClientIP == "" && iClientPort == peerPort()) ||
        (sClientName == peerName() && sClientIP == peerAddress().toString() && iClientPort == 0) ||
        (sClientName == peerName() && sClientIP == "" && iClientPort == peerPort()) ||
        (sClientName == "" && sClientIP == peerAddress().toString() && iClientPort == peerPort()) ||
        (sClientName == peerName() && sClientIP == peerAddress().toString() && iClientPort == peerPort()) ) {
        //Send text to remote, using UTF-8
        write(sDataToSend.toUtf8());
    }
    return;
}

/* Command Incoming Event Handler Slot */
void TCPServerSocket::CommandReceivedFromClientEventHandler() {
    while (bytesAvailable()) {
        //Read a command line and emit a signal
        emit SocketCommandReceivedFromClientEvent(readLine(), peerName(), peerAddress().toString(), peerPort());

        //Process events
        //QApplication::processEvents();
    }
    return;
}

/* TCP Socket Event Handler Slots */
void TCPServerSocket::TCPServerSocket_Connected() {
    qDebug() << "TCPServer: Connection established with remote client" << peerAddress() << ":" << peerPort() << ".";
    emit SocketConnectedToClientEvent(peerName(), peerAddress().toString(), peerPort());
    return;
}

void TCPServerSocket::TCPServerSocket_Disconnected() {
    qDebug() << "TCPServer: Remote client" << peerAddress() << "disconnected.";
    emit SocketDisconnectedFromClientEvent(peerName(), peerAddress().toString(), peerPort());
    this->deleteLater(); //Delete this object safely
    return;
}

void TCPServerSocket::TCPServerSocket_Error(QAbstractSocket::SocketError errErrorInfo) {
    qDebug() << "TCPServer: Error" << errErrorInfo << "occurred, connection aborted.";
    emit SocketErrorOccurredEvent(errErrorInfo, peerName(), peerAddress().toString(), peerPort());
    this->deleteLater(); //Delete this object safely
    return;
}

/* TCP Server Object */
TCPServer::TCPServer() {
    //Load settings
    TCPServer::LoadSettings();
}

TCPServer::TCPServer(quint16 iListeningPortInit) {
    //Save settings
    iListeningPort = iListeningPortInit;
    TCPServer::SaveSettings();
}

TCPServer::~TCPServer() {
    //Save settings
    TCPServer::SaveSettings();

    //Close server
    if (isListening()) {
        TCPServer::StopListening();
    }
}

/* Options Management */
void TCPServer::LoadSettings() {
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    iListeningPort = SettingsContainer.value(ST_KEY_LISTENING_PORT, ST_DEFVAL_LISTENING_PORT).toUInt();
    SettingsContainer.endGroup();
    return;
}

void TCPServer::SaveSettings() const {
    SettingsContainer.beginGroup(ST_KEY_NETWORKING_PREFIX);
    SettingsContainer.setValue(ST_KEY_LISTENING_PORT, iListeningPort);
    SettingsContainer.endGroup();
    return;
}


/* Listening Status Management */
bool TCPServer::StartListening() {
    if (listen(QHostAddress::Any, iListeningPort)) {
        qDebug() << "TCPServer: Started listening on port" << iListeningPort;
        return true;
    }
    else {
        qDebug() << "TCPServer: Couldnot start listening on port" << iListeningPort;
        return false;
    }
    return false;
}

bool TCPServer::StartListening(quint16 iListeningPortNew) {
    //Save settings
    iListeningPort = iListeningPortNew;
    TCPServer::SaveSettings();

    //Try starting listening
    if (listen(QHostAddress::Any, iListeningPort)) {
        qDebug() << "TCPServer: Started listening on port" << iListeningPort;
        return true;
    }
    else {
        qDebug() << "TCPServer: Couldnot start listening on port" << iListeningPort;
        return false;
    }
    return false;
}

void TCPServer::StopListening() {
    qDebug() << "TCPServer: Server closed";
    close(); //Close the sever and stop listening
    return;
}

/* Text-Based Communication */
void TCPServer::SendDataToClient(QString sDataToSend, QString sClientName, QString sClientIP, quint16 iClientPort) {
    emit SendDataToClientRequestedEvent(sDataToSend, sClientName, sClientIP, iClientPort);
}

/* Command Incoming Event Handler Slot */
void TCPServer::SocketCommandReceivedFromClientEventHandler(QString sCommand, QString sClientName, QString sClientIP, quint16 iClientPort) {
    qDebug() << "TCPServer: Command" << sCommand << "received from the remote";
    emit CommandReceivedEvent(sCommand, sClientName, sClientIP, iClientPort);
    return;
}

/* Incoming Connection Management */
void TCPServer::incomingConnection(int iSocketID) {
    //Create a new socket object
    TCPServerSocket * tcpSocket = new TCPServerSocket;
    tcpSocket->setSocketDescriptor(iSocketID);

    //Connect events and handlers
    connect(tcpSocket, SIGNAL(SocketConnectedToClientEvent(QString, QString, quint16)), this, SIGNAL(ClientConnectedEvent(QString, QString, quint16)));
    connect(tcpSocket, SIGNAL(SocketDisconnectedFromClientEvent(QString, QString, quint16)), this, SIGNAL(ClientDisconnectedEvent(QString, QString, quint16)));
    connect(tcpSocket, SIGNAL(SocketErrorOccurredEvent(QAbstractSocket::SocketError, QString, QString, quint16)), this, SIGNAL(ClientNetworkingErrorOccurredEvent(QAbstractSocket::SocketError, QString, QString, quint16)));
    connect(tcpSocket, SIGNAL(SocketCommandReceivedFromClientEvent(QString, QString, QString, quint16)), this, SLOT(SocketCommandReceivedFromClientEventHandler(QString, QString, QString, quint16)));
    connect(this, SIGNAL(SendDataToClientRequestedEvent(QString, QString, QString, quint16)), tcpSocket, SLOT(SendDataToClientRequestedEventHandler(QString, QString, QString, quint16)));

    return;
}
