#include "scraper.h"

#include <QStringList>

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>

#include <curl/curl.h>

Scraper::Scraper()
{
}

int Scraper::randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

QString Scraper::getRandomUserAgent()
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

//size_t Scraper::curl_write_data(void *data, size_t size, size_t nmemb,
//                                        void *pointer)
//{
//    size_t data_size = size * nmemb;

//    QByteArray* ptr=(QByteArray*)pointer;

//    ptr->append(QByteArray::fromRawData((const char*)data,data_size));

//    return data_size;
//}

//size_t Scraper::curl_header(void *data, size_t size, size_t nmemb,
//                                    void *pointer)
//{
//    size_t data_size = size * nmemb;

//    QByteArray* ptr=(QByteArray*)pointer;

//    ptr->append(QByteArray::fromRawData((const char*)data,data_size));

//    return data_size;
//}

//bool Scraper::execCommand(const QString& urlCommand, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const{

//    bool result=false;

//    QString userAgent=getRandomUserAgent();

//    QString ip = QString("%1.%1.%1.%1").arg(randInt(0, 255),randInt(0, 255),randInt(0, 255),randInt(0, 255));

//    CURL *curl;
//    CURLcode res;

//    curl = curl_easy_init();
//    if(curl) {
//        QString url=createURL(urlCommand,params);

//        char errorBuffer[CURL_ERROR_SIZE];
//        memset(errorBuffer,0,CURL_ERROR_SIZE);

//        curl_easy_setopt(curl, CURLOPT_URL, qPrintable(url));
//        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
//        // curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, true);
//        curl_easy_setopt(curl, CURLOPT_USERAGENT, qPrintable(userAgent));
//     //   curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

//        struct curl_slist *headers=NULL; // init to NULL is important
//        curl_slist_append(headers, qPrintable(QString("REMOTE_ADDR: %1").arg(ip)));
//        curl_slist_append(headers, qPrintable(QString("HTTP_X_FORWARDED_FOR: %1").arg(ip)));
//        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
//        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header);

//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
//        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

//        /* Perform the request, res will get the return code */
//        res = curl_easy_perform(curl);

//        if (res==0) {
//            if (headerData.startsWith("HTTP/1.1 200 OK")){
//                result=true;
//            } else {
//                qDebug() << headerData << data;
//            }
//        } else {
//            qDebug("HttpStreamReader: curl thread finished with code %d (%s)", res, errorBuffer);
//        }
//        /* always cleanup */
//        curl_easy_cleanup(curl);
//    }

//    return result;
// }
