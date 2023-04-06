#include "MainWindow.h"
#include "NetworkingControlInterface.h"
#include "ui_MainWindow.h"
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget * parent, QString sHostIP, quint16 iHostPort) : QMainWindow(parent),
                                                                               ui(new Ui::MainWindow) {
    ui->setupUi(this);

    /* TCP Client Object */
    tcpDataClient = new TCPClient;
    connect(tcpDataClient, SIGNAL(ResponseReceivedFromServerEvent(QString, QString, QString, quint16)), this, SLOT(ResponseReceivedEventHandler(QString, QString, QString, quint16)));
    connect(tcpDataClient, SIGNAL(ConnectedToServerEvent(QString, QString, quint16)), this, SLOT(ConnectedToServerEventHandler(QString, QString, quint16)));
    connect(tcpDataClient, SIGNAL(DisconnectedFromServerEvent(QString, QString, quint16)), this, SLOT(DisconnectedFromServerEventHandler(QString, QString, quint16)));
    connect(tcpDataClient, SIGNAL(NetworkingErrorOccurredEvent(QAbstractSocket::SocketError, QString, QString, quint16)), this, SLOT(NetworkingErrorOccurredEventHandler(QAbstractSocket::SocketError, QString, QString, quint16)));

    /* TCP Server Object */
    tcpCommandServer = new TCPServer;
    connect(tcpCommandServer, SIGNAL(ClientConnectedEvent(QString, QString, quint16)), this, SLOT(ClientConnectedEventHandler(QString, QString, quint16)));
    connect(tcpCommandServer, SIGNAL(ClientDisconnectedEvent(QString, QString, quint16)), this, SLOT(ClientDisconnectedEventHandler(QString, QString, quint16)));
    connect(tcpCommandServer, SIGNAL(ClientNetworkingErrorOccurredEvent(QAbstractSocket::SocketError, QString, QString, quint16)), this, SLOT(ClientNetworkingErrorOccurredEventHandler(QAbstractSocket::SocketError, QString, QString, quint16)));
    connect(tcpCommandServer, SIGNAL(CommandReceivedEvent(QString, QString, QString, quint16)), this, SLOT(DataReceivedFromClientEventHandler(QString, QString, QString, quint16)));
    tcpCommandServer->StartListening();

    /* Heart Beat Timer */
    tmrHeartBeat = new QTimer(this);
    connect(tmrHeartBeat, SIGNAL(timeout()), this, SLOT(tmrHeartBeat_Tick()));
    tmrHeartBeat->start(5000);

    /* Establish connection */
    if (sHostIP == "") {
        sHostIP = tcpDataClient->GetServerIP();
    }
    if (iHostPort == 0) {
        iHostPort = tcpDataClient->GetServerPort();
    }
    tcpDataClient->ConnectToServer(sHostIP, iHostPort, true, 2000);
}

MainWindow::~MainWindow() {
    /* Close Networking */
    tcpDataClient->DisconnectFromServer();
    tcpCommandServer->StopListening();

    tmrHeartBeat->stop();
    delete tmrHeartBeat;

    /* Delete TCP Client Object */
    delete tcpDataClient;

    /* Delete TCP Server Object */
    delete tcpCommandServer;

    delete ui;
}

void MainWindow::WriteLog(const QString & sLog, bool bIsSeparatorRequired) {
    ui->txtHistory->appendPlainText(sLog);
    if (bIsSeparatorRequired) {
        ui->txtHistory->appendPlainText("");
    }
}

/* Networking Events Handler */
void MainWindow::ConnectedToServerEventHandler(QString sServerName, QString sServerIPAddress, quint16 iServerPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Connected to server \"" + sServerName + "\" (" + sServerIPAddress + ":" + QString::number(iServerPort) + ")", true);
    return;
}

void MainWindow::DisconnectedFromServerEventHandler(QString sServerName, QString sServerIPAddress, quint16 iServerPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Disconnected from server \"" + sServerName + "\" (" + sServerIPAddress + ":" + QString::number(iServerPort) + ")", true);
    return;
}

void MainWindow::NetworkingErrorOccurredEventHandler(QAbstractSocket::SocketError errErrorInfo, QString sServerName, QString sServerIPAddress, quint16 iServerPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Network error with server \"" + sServerName + "\" (" + sServerIPAddress + ":" + QString::number(iServerPort) + "): " + QString::number(errErrorInfo), true);
    return;
}

void MainWindow::ResponseReceivedEventHandler(QString sResponse, QString sServerName, QString sServerIPAddress, quint16 iServerPort) {
    WriteLog("Server \"" + sServerName + "\" (" + sServerIPAddress + ":" + QString::number(iServerPort) + ") @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog(sResponse, true);
    return;
}

void MainWindow::ClientConnectedEventHandler(QString sClientName, QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Client \"" + sClientName + "\" (" + sClientIPAddress + ":" + QString::number(iClientPort) + ") connected", true);
    return;
}

void MainWindow::ClientDisconnectedEventHandler(QString sClientName, QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Client \"" + sClientName + "\" (" + sClientIPAddress + ":" + QString::number(iClientPort) + ") disconnected", true);
    return;
}

void MainWindow::ClientNetworkingErrorOccurredEventHandler(QAbstractSocket::SocketError errErrorInfo, QString sClientName, QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Network error with client \"" + sClientName + "\" (" + sClientIPAddress + ":" + QString::number(iClientPort) + "): " + QString::number(errErrorInfo), true);
    return;
}

void MainWindow::DataReceivedFromClientEventHandler(QString sData, QString sClientName, QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("Client \"" + sClientName + "\" (" + sClientIPAddress + ":" + QString::number(iClientPort) + ") @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog(sData, true);
    return;
}

/* Heart Beat Timer Slot */
void MainWindow::tmrHeartBeat_Tick() {
    tcpDataClient->QueueDataFrame("<HEART BEAT MESSAGE>");
    WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("<HEART BEAT MESSAGE>", true);
    if (tcpDataClient->IsConnected()) {
        tcpDataClient->SendDataToServer();
    }
    tcpCommandServer->SendDataToClient("<HEART BEAT MESSAGE>");
}

/* Widget Intercation Events */
void MainWindow::on_btnSend_clicked() {
    if (ui->txtInput->document()->toPlainText() != "") {
        tcpDataClient->QueueDataFrame(ui->txtInput->document()->toPlainText());
        WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
        WriteLog(ui->txtInput->document()->toPlainText(), true);
        if (tcpDataClient->IsConnected()) {
            tcpDataClient->SendDataToServer();
        }
        tcpCommandServer->SendDataToClient(ui->txtInput->document()->toPlainText());
    }
    else {
        tcpDataClient->QueueDataFrame("<EMPTY TEXT>");
        WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
        WriteLog("<EMPTY TEXT>", true);
        if (tcpDataClient->IsConnected()) {
            tcpDataClient->SendDataToServer();
        }
        tcpCommandServer->SendDataToClient("<EMPTY TEXT>");
    }
    return;
}

void MainWindow::on_btnClose_clicked() {
    qApp->quit();
    //exit(0);
    return;
}
