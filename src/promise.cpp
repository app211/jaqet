#include "promise.h"

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

Promise::Promise()
{
    connect(this, &Promise::completed, this, &Promise::deleteLater);
}

Promise* Promise::loadAsync(QNetworkAccessManager & manager, const QString& url){

       // Fetch a large file
       //  QNetworkRequest request(QUrl(url));
       QNetworkReply * reply = manager.get(QNetworkRequest(QUrl(url)));

       Promise * promise = new Promise;
       promise->reply=reply;

       // When the promise is canceled, abort the request
       QObject::connect(promise, &Promise::canceled, [reply]()
       {
           reply->abort();
       });

       // When the request finishes (or is aborted), indicate that the promise completed
       QObject::connect(reply, &QNetworkReply::finished, promise, &Promise::completed);

       return promise;

   }
