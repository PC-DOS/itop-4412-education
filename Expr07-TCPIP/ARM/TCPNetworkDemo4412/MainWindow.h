#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, QString sHostIP="", quint16 iHostPort=0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void WriteLog(const QString & sLog, bool IsSeparatorRequired=false);

    QTimer * tmrHeartBeat;

private slots:
    /* Response from Server Handler */
    void ResponseReceivedEventHandler(QString sResponse);
    void on_btnSend_clicked();
    void on_btnClose_clicked();

    /* Heart Beat Timer Slot */
    void tmrHeartBeat_Tick();
};

#endif // MAINWINDOW_H
