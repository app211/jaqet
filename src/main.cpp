#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QStyle>

static QtMessageHandler oldHandler = nullptr;
static QFile outFile("LogFile.log");
static QTextStream textStream;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    #ifdef QT_DEBUG
        if (oldHandler){
            (*oldHandler)( type, context, msg);
        }
    #endif


    textStream << QString("[%1] ").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));

    switch (type)
    {
    case QtDebugMsg:
        textStream << QString("{Debug} \t\t %1").arg(msg) << endl;
        break;
    case QtWarningMsg:
        textStream << QString("{Warning} \t %1").arg(msg) << endl;
        break;
    case QtCriticalMsg:
        textStream << QString("{Critical} \t %1").arg(msg) << endl;
        break;
    case QtFatalMsg:
        textStream << QString("{Fatal} \t\t %1").arg(msg) << endl;
        abort();
        break;
    }
}

int main(int argc, char *argv[])
{
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);

    textStream.setDevice(&outFile);

    oldHandler=qInstallMessageHandler(customMessageHandler);

    QApplication app(argc, argv);


    qsrand(QTime::currentTime().msec()) ;

    QLoggingCategory::setFilterRules(LOGRULES);

    qDebug() << "Starting ....";
    qDebug() << QApplication::style()->metaObject()->className();
    qDebug() << QApplication::style()->objectName();

    QString style;
    QFile styleFile( ":/resources/styles/default.qss" );
    if (styleFile.open( QFile::ReadOnly )){
        style.append( styleFile.readAll() );
    }

    QFile systemStyleFile(":/resources/styles/"+QApplication::style()->objectName()+".qss");
    if (systemStyleFile.open( QFile::ReadOnly )){
        style.append(systemStyleFile.readAll());
    }

    if (!style.isEmpty()){
        app.setStyleSheet( style );
    }


    MainWindow w;
    w.show();

    int ret= app.exec();

    qInstallMessageHandler(0);

    outFile.close();

    return ret;
}
