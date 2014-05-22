#ifndef CURLACCESS_H
#define CURLACCESS_H

#include "httpaccess.h"



class CurlAccess : public HttpAccess
{
    static size_t curl_write_data(void *data, size_t size, size_t nmemb, void *pointer);
    static size_t curl_header(void *data, size_t size, size_t nmemb, void *pointer);

public:
    CurlAccess();
    virtual bool synchroPost(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const;
    virtual bool synchroGet(const QUrl& url, QMap<QString,QString>& params, QByteArray& headerData, QByteArray& data) const;
};

#endif // CURLACCESS_H
