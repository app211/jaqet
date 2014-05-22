#ifndef PROMISE_H
#define PROMISE_H

#include <QObject>

class QNetworkReply;
class QNetworkAccessManager;

class Promise : public QObject
{
    Q_OBJECT

    public:
        Promise();
 QNetworkReply * reply;
 static Promise* loadAsync(QNetworkAccessManager & manager, const QString& url);

    Q_SIGNALS:
        void completed();
        void canceled();

};

#endif // PROMISE_H
