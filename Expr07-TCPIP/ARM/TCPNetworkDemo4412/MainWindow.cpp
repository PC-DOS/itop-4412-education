#include <QDateTime>
#include <QDebug>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NetworkingControlInterface.h"

MainWindow::MainWindow(QWidget *parent, QString sHostIP, quint16 iHostPort) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* TCP Client Object */
    tcpDataClient=new TCPClient;
    connect(tcpDataClient, SIGNAL(ResponseReceivedFromServerEvent(QString)), this, SLOT(ResponseReceivedEventHandler(QString)), Qt::QueuedConnection);

    /* Heart Beat Timer */
    tmrHeartBeat=new QTimer(this);
    connect(tmrHeartBeat, SIGNAL(timeout()), this, SLOT(tmrHeartBeat_Tick()));
    tmrHeartBeat->start(5000);

    /* Establish connection */
    if (sHostIP==""){
        sHostIP=tcpDataClient->GetServerIP();
    }
    if (iHostPort==0){
        iHostPort=tcpDataClient->GetServerPort();
    }
    tcpDataClient->ConnectToServer(sHostIP,iHostPort,true,2000);
}

MainWindow::~MainWindow(){
    tmrHeartBeat->stop();
    delete tmrHeartBeat;

    /* Delete TCP Client Object */
    delete tcpDataClient;
    delete ui;
}

void MainWindow::WriteLog(const QString & sLog, bool bIsSeparatorRequired){
    ui->txtHistory->appendPlainText(sLog);
    if (bIsSeparatorRequired){
        ui->txtHistory->appendPlainText("");
    }
}

void MainWindow::ResponseReceivedEventHandler(QString sResponse){
    WriteLog("Server " + tcpDataClient->GetServerIP() + ":" + QString::number(tcpDataClient->GetServerPort()) + " @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog(sResponse, true);
    return;
}

void MainWindow::on_btnSend_clicked(){
    if (ui->txtInput->document()->toPlainText() != ""){
        tcpDataClient->QueueDataFrame(ui->txtInput->document()->toPlainText());
        WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
        WriteLog(ui->txtInput->document()->toPlainText(), true);
        if (tcpDataClient->IsConnected()){
            tcpDataClient->SendDataToServer();
        }
    }
    else{
        tcpDataClient->QueueDataFrame("<EMPTY TEXT>");
        WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
        WriteLog("<EMPTY TEXT>", true);
        if (tcpDataClient->IsConnected()){
            tcpDataClient->SendDataToServer();
        }
    }
    return;
}

void MainWindow::on_btnClose_clicked(){
    QApplication::quit();
    return;
}

void MainWindow::tmrHeartBeat_Tick(){
    tcpDataClient->QueueDataFrame("<HEART BEAT MESSAGE>");
    WriteLog("Me @ " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ":");
    WriteLog("<HEART BEAT MESSAGE>", true);
    if (tcpDataClient->IsConnected()){
        tcpDataClient->SendDataToServer();
    }
}
