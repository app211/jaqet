#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile( ":/resources/styles/CheckLock.qss" );
    styleFile.open( QFile::ReadOnly );

    // Apply the loaded stylesheet
    QString style( styleFile.readAll() );
    app.setStyleSheet( style );

    qsrand(QTime::currentTime().msec()) ;

    MainWindow w;
    w.show();


    return app.exec();
}
