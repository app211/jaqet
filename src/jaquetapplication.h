#ifndef JAQUETAPPLICATION_H
#define JAQUETAPPLICATION_H

#include <QApplication>
#include <QFile>
#include <QTextStream>

class JaquetApplication : public QApplication
{
    static QtMessageHandler oldHandler;
    static QFile outFile;
    static QTextStream textStream;

public:
    JaquetApplication(int& argc, char** argv);
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    int go();
};

#endif // JAQUETAPPLICATION_H
