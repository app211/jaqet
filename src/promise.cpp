#include "promise.h"

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

int randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

QString getRandomUserAgent()
{

    QString v=QString("%1").arg(randInt(1, 4))+'.'+QString("%1").arg(randInt(0, 9));
    QString a=QString("%1").arg(randInt(0, 9));
    QString b=QString("%1").arg(randInt(0, 99));
    QString c=QString("%1").arg(randInt(0, 999));

    QStringList agents;
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; fr-fr; Nexus One Build/FRF91) AppleWebKit/5%3.%4 (KHTML, like Gecko) Version/%2.%2 Mobile Safari/5%3.%4").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; fr-fr; Dell Streak Build/Donut AppleWebKit/5%3.%4+ (KHTML, like Gecko) Version/3.%2.2 Mobile Safari/ 5%3.%4.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 4.%1; fr-fr; LG-L160L Build/IML74K) AppleWebkit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 4.%1; fr-fr; HTC Sensation Build/IML74K) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; en-gb) AppleWebKit/999+ (KHTML, like Gecko) Safari/9%3.%2").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.5; fr-fr; HTC_IncredibleS_S710e Build/GRJ%3) AppleWebKit/5%3.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/5%3.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC Vision Build/GRI%3) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.4; fr-fr; HTC Desire Build/GRJ%3) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; T-Mobile myTouch 3G Slide Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.3; fr-fr; HTC_Pyramid Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC_Pyramid Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC Pyramid Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/5%3.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; LG-LU3000 Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/5%3.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC_DesireS_S510e Build/GRI%2) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/%4.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC_DesireS_S510e Build/GRI40) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.3; fr-fr; HTC Desire Build/GRI%2) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android 2.%1; fr-fr; HTC Desire Build/FRF%2) AppleWebKit/533.1 (KHTML, like Gecko) Version/%2.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; fr-lu; HTC Legend Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/%2.%2 Mobile Safari/%4.%2").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; fr-fr; HTC_DesireHD_A9191 Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.1; fr-fr; HTC_DesireZ_A7%4 Build/FRG83D) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/%4.%2").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1.1; en-gb; HTC_DesireZ_A7272 Build/FRG83D) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/%4.1").arg(v).arg(a).arg(b).arg(c));
    agents.append(QString("Mozilla/5.0 (Linux; U; Android %1; fr-fr; LG-P5%3 Build/FRG83) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1").arg(v).arg(a).arg(b).arg(c));

    return agents.at(randInt(0,agents.size()-1));
}

Promise::Promise()
{
    connect(this, &Promise::completed, this, &Promise::complete);
}

void Promise::complete(){
    deleteLater();
}

Promise* Promise::loadAsync(QNetworkAccessManager & manager, const QString& url, bool randomIP){

    // Fetch a large file
    QString ip = QString("%1.%1.%1.%1").arg(randInt(0, 255),randInt(0, 255),randInt(0, 255),randInt(0, 255));

    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setRawHeader( "User-Agent" , getRandomUserAgent().toLatin1());
    if (randomIP){
        req.setRawHeader("X-Forwarded-For", ip.toLatin1());
        req.setRawHeader("Client-IP", ip.toLatin1());
        req.setRawHeader("VIA", ip.toLatin1());
    }

    QNetworkReply * reply = manager.get(req);

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
