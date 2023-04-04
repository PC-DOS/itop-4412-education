#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "NetworkingControlInterface.h"
#include <QMainWindow>
#include <QTimer>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = 0, QString sHostIP = "", quint16 iHostPort = 0);
    ~MainWindow();

private:
    Ui::MainWindow * ui;
    void WriteLog(const QString & sLog, bool bIsSeparatorRequired = false);

    QTimer * tmrHeartBeat;

private slots:
    /* Networking Events Handler */
    void ConnectedToServerEventHandler();
    void DisconnectedFromServerEventHandler();
    void NetworkingErrorEventHandler(QAbstractSocket::SocketError errErrorInfo);
    void ResponseReceivedEventHandler(QString sResponse);
    void ClientConnectedEventHandler(QString sClientIPAddress, quint16 iClientPort);
    void DataReceivedFromClientEventHandler(QString sData, QString sClientIPAddress, quint16 iClientPort);

    /* Heart Beat Timer Slot */
    void tmrHeartBeat_Tick();

    /* Widget Intercation Events */
    void on_btnSend_clicked();
    void on_btnClose_clicked();
};

#endif // MAINWINDOW_H
