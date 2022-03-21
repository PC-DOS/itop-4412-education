#include "MainWindow.h"
#include <QApplication>
#include <QString>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    /* Parse command */
    QString sHostIPParam="";
    quint16 iHostPortParam=0;
    switch (argc){
    case 1:
        sHostIPParam="";
        iHostPortParam=0;
        break;
    case 2:
        sHostIPParam=QString::fromAscii(argv[1]);
        iHostPortParam=0;
        break;
    case 3:
        sHostIPParam=QString::fromAscii(argv[1]);
        iHostPortParam=QString::fromAscii(argv[2]).toInt();
    }

    MainWindow w(NULL, sHostIPParam, iHostPortParam);
    w.show();

    return a.exec();
}
