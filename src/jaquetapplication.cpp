#include "jaquetapplication.h"

#include <QFile>
#include <QStyle>
#include <QDebug>
#include <QDebug>
#include <QtGlobal>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QStyle>
#include "src/widgets/jaqetmainwindow.h"
#include "jaquetapplication.h"

QtMessageHandler JaquetApplication::oldHandler = nullptr;
QFile JaquetApplication::outFile("LogFile.log");
QTextStream JaquetApplication::textStream;

void JaquetApplication::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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


JaquetApplication::JaquetApplication(int& argc, char** argv) : QApplication(argc, argv){

    if (!outFile.isOpen()){
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    if (!textStream.device()){
        textStream.setDevice(&outFile);
    }

    if (!oldHandler){
        oldHandler=qInstallMessageHandler(customMessageHandler);
    }
}

int JaquetApplication::go(){

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
        setStyleSheet( style );
    }

    qsrand(QTime::currentTime().msec()) ;

    QLoggingCategory::setFilterRules(LOGRULES);

    QScopedPointer<JaqetMainWindow> mainWindow(JaqetMainWindow::getInstance());
    mainWindow->show();

    int ret= exec();

    qInstallMessageHandler(0);

    return ret;
}
