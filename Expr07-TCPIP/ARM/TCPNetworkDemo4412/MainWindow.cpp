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
    connect(tcpDataClient, SIGNAL(ResponseReceivedFromServerEvent(QString)), this, SLOT(ResponseReceivedEventHandler(QString)));
    connect(tcpDataClient, SIGNAL(ConnectedToServerEvent()), this, SLOT(ConnectedToServerEventHandler()));
    connect(tcpDataClient, SIGNAL(DisconnectedFromServerEvent()), this, SLOT(DisconnectedFromServerEventHandler()));
    connect(tcpDataClient, SIGNAL(NetworkingErrorEvent(QAbstractSocket::SocketError)), this, SLOT(NetworkingErrorEventHandler(QAbstractSocket::SocketError)));

    /* TCP Server Object */
    tcpCommandServer = new TCPServer;
    connect(tcpCommandServer, SIGNAL(ClientConnectedEvent(QString, quint16)), this, SLOT(ClientConnectedEventHandler(QString, quint16)));
    connect(tcpCommandServer, SIGNAL(CommandReceivedEvent(QString, QString, quint16)), this, SLOT(DataReceivedFromClientEventHandler(QString, QString, quint16)));
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
void MainWindow::ConnectedToServerEventHandler() {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Connected to server " + tcpDataClient->GetServerIP() + ":" + QString::number(tcpDataClient->GetServerPort()), true);
    return;
}

void MainWindow::DisconnectedFromServerEventHandler() {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Disconnected from server " + tcpDataClient->GetServerIP() + ":" + QString::number(tcpDataClient->GetServerPort()), true);
    return;
}

void MainWindow::NetworkingErrorEventHandler(QAbstractSocket::SocketError errErrorInfo) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Network error: " + QString::number(errErrorInfo), true);
    return;
}

void MainWindow::ResponseReceivedEventHandler(QString sResponse) {
    WriteLog("Server " + tcpDataClient->GetServerIP() + ":" + QString::number(tcpDataClient->GetServerPort()) + " @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog(sResponse, true);
    return;
}

void MainWindow::ClientConnectedEventHandler(QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("System @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("Client " + sClientIPAddress + ":" + QString::number(iClientPort) + " connected", true);
    return;
}

void MainWindow::DataReceivedFromClientEventHandler(QString sData, QString sClientIPAddress, quint16 iClientPort) {
    WriteLog("Client " + sClientIPAddress + ":" + QString::number(iClientPort) + " @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
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
