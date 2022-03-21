#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    /* Response from Server Handler */
    void ResponseReceivedEventHandler(QString sResponse);
    void on_btnSend_clicked();
    void on_btnClose_clicked();
};

#endif // MAINWINDOW_H
