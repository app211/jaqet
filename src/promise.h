#ifndef PROMISE_H
#define PROMISE_H

#include <QObject>

#include<QNetworkReply>

class QNetworkAccessManager;

class Promise : public QObject
{
    Q_OBJECT

public:
    Promise();
    QNetworkReply * reply=nullptr;
    static Promise* loadAsync( QNetworkAccessManager & manager, const QString& url, bool useRandomIP=true, bool useRandomUserAgent=true, QNetworkRequest::Priority priority=QNetworkRequest::NormalPriority);

public slots:
    void complete();
    void cancel();

Q_SIGNALS:
    void completed();
    void canceled();

};

#endif // PROMISE_H
