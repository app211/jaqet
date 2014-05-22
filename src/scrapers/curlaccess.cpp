#include "curlaccess.h"

#include <QDebug>
#include <QUrl>
#include <curl/curl.h>

CurlAccess::CurlAccess()
{
}

size_t CurlAccess::curl_write_data(void *data, size_t size, size_t nmemb,
                                   void *pointer)
{
    size_t data_size = size * nmemb;

    QByteArray* ptr=(QByteArray*)pointer;

    ptr->append(QByteArray::fromRawData((const char*)data,data_size));

    return data_size;
}

size_t CurlAccess::curl_header(void *data, size_t size, size_t nmemb,
                               void *pointer)
{
    size_t data_size = size * nmemb;

    QByteArray* ptr=(QByteArray*)pointer;

    ptr->append(QByteArray::fromRawData((const char*)data,data_size));

    return data_size;
}

bool CurlAccess::synchroPost(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const{
    bool result=false;

    QString userAgent=getRandomUserAgent();

    QString ip = getRandomIP();

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {

        char errorBuffer[CURL_ERROR_SIZE];
        memset(errorBuffer,0,CURL_ERROR_SIZE);

        curl_easy_setopt(curl, CURLOPT_URL, qPrintable(url.toString()));
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, qPrintable(userAgent));

        struct curl_slist *headers=NULL; // init to NULL is important
        curl_slist_append(headers, qPrintable(QString("REMOTE_ADDR: %1").arg(ip)));
        curl_slist_append(headers, qPrintable(QString("HTTP_X_FORWARDED_FOR: %1").arg(ip)));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_POST, 1);

        QString mapParams;

        foreach( QString key, params.keys() )
        {
            if (mapParams.size()>0){
                mapParams.append('&');
            }
            mapParams.append(QUrl::toPercentEncoding(key)).append('=').append(params.value( QUrl::toPercentEncoding(key) ));
        }

        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, qPrintable(mapParams));

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        if (res==0) {
            if (headerData.startsWith("HTTP/1.1 200 OK")){
                //     qDebug() << headerData << data;
                result=true;
            } else {
                qDebug() << headerData << data;
            }
        } else {
            qDebug("HttpStreamReader: curl thread finished with code %d (%s)", res, errorBuffer);
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return result;
}

bool CurlAccess::synchroGet(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const{
    bool result=false;

    QString userAgent=getRandomUserAgent();

    QString ip = getRandomIP();

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {

        char errorBuffer[CURL_ERROR_SIZE];
        memset(errorBuffer,0,CURL_ERROR_SIZE);

        curl_easy_setopt(curl, CURLOPT_URL, qPrintable(url.toString()));
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, qPrintable(userAgent));

        struct curl_slist *headers=NULL; // init to NULL is important
        curl_slist_append(headers, qPrintable(QString("REMOTE_ADDR: %1").arg(ip)));
        curl_slist_append(headers, qPrintable(QString("HTTP_X_FORWARDED_FOR: %1").arg(ip)));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        QString mapParams;

        foreach( QString key, params.keys() )
        {
            if (mapParams.size()>0){
                mapParams.append('&');
            }
            mapParams.append(QUrl::toPercentEncoding(key)).append('=').append(params.value( QUrl::toPercentEncoding(key) ));
        }


        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        if (res==0) {
            if (headerData.startsWith("HTTP/1.1 200 OK")){
                //     qDebug() << headerData << data;
                result=true;
            } else {
                qDebug() << headerData << data;
            }
        } else {
            qDebug("HttpStreamReader: curl thread finished with code %d (%s)", res, errorBuffer);
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return result;
}
