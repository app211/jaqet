#ifndef PROMISE_H
#define PROMISE_H

#include <QObject>

#include<QPointer>
#include<QNetworkReply>

class QNetworkAccessManager;

class Promise : public QObject
{
    Q_OBJECT

    QPointer<QNetworkReply> reply;

public:
    Promise();
     static Promise* loadAsync( QNetworkAccessManager & manager, const QString& url, bool useRandomIP=true, bool useRandomUserAgent=true, QNetworkRequest::Priority priority=QNetworkRequest::NormalPriority);

     QNetworkReply::NetworkError replyError();
     QString replyErrorString();
     QByteArray replyData();

public slots:
    void complete();
    void cancel();

Q_SIGNALS:
    void completed();
    void canceled();

};

#endif // PROMISE_H
