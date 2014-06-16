#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qsrand(QTime::currentTime().msec()) ;

    MainWindow w;
    w.show();


    return a.exec();
}
