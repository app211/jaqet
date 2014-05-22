#ifndef HTTPACCESS_H
#define HTTPACCESS_H

#include <QString>
#include <QMap>
#include <QByteArray>
#include <QUrl>

class HttpAccess
{
protected:
    static int randInt(int low, int high) ;
    static QStringList agents;
    static QString getRandomUserAgent() ;
    static QString getRandomIP();
public:
    HttpAccess();

    virtual bool synchroPost(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const=0;
    virtual bool synchroGet(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const=0;

};

#endif // HTTPACCESS_H
