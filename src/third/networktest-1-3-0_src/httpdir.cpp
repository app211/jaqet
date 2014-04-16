// -------------------------------------------------------------------------------
//  Project:		Qt Webbased File Handling
//
//  File:           httpdir.cpp
//
// (c) 2012 - 2013 by Oblivion Software/Norbert Schlia
// This source code is licensed under The Code Project Open License (CPOL),
// see http://www.codeproject.com/info/cpol10.aspx
// -------------------------------------------------------------------------------
//
#include "httpdir.h"

#include "webfile.h"
#include "entities.h"

#include <QDebug>

//httpdir::httpdir(const httpdir & httpdir)
//  : m_pNetworkProxy(NULL)
//{
//    *this = httpdir;
//}

httpdir::httpdir(const QUrl & url)
    : m_Url(url)
{
    m_pNetworkProxy = NULL;
    refresh();
}

httpdir::httpdir(const QString & path /*= QString()*/)
{
    m_pNetworkProxy = NULL;
    if (!path.isEmpty())
    {
        setUrl(path);
    }
}

//httpdir::httpdir(const QString &path, const QString &nameFilter, QDir::SortFlags sort /*= QDir::SortFlags(QDir::Name | QDir::IgnoreCase)*/, QDir::Filters filter /*= QDir::AllEntries*/)
//    : m_pNetworkProxy(NULL)
//    , m_Url(path)
//{
//}

httpdir::~httpdir()
{
}

httpdir & httpdir::operator=(const httpdir & httpdir)
{
    if (this != &httpdir)  // or if (*this != io)
    {
        m_Url                   = httpdir.m_Url;
    }

    return *this;
}

void httpdir::setUrl(const QString & path)
{
    m_Url = path;
    refresh();
}

void httpdir::setUrl(const QUrl & url)
{
    m_Url = url;
    refresh();
}

QUrl httpdir::url() const
{
    return m_Url;
}

void httpdir::refresh()
{
    webfile file(m_Url.toString());

    file.setUser(m_strUserName);
    file.setPassword(m_strPassword);

    if (m_pNetworkProxy != NULL)
    {
        file.setProxy(m_pNetworkProxy);
    }

    m_HttpFileInfoList.clear();

    m_strErrorString.clear();
    m_Error = QNetworkReply::NoError;

    qDebug() << m_Url;

    if (file.open())
    {
        do
        {
            char szLine[1024] = "";
            qint64 i64Read = file.readLine(szLine, sizeof(szLine));
            if (i64Read <= 0)
            {
                break;
            }

            QString strLine(szLine);
            httpfileinfo httpfile;

            decode_html_entities(strLine);

            if (httpfile.matchLine(m_Url.toString(), strLine))
            {
                m_HttpFileInfoList.append(httpfile);
            }
        } while (true);

        file.close();
    }
    else
    {
        m_strErrorString = file.errorString();
        m_Error = file.error();
    }
}

httpfileinfolist httpdir::entryInfoList(QDir::Filters filters /*= QDir::NoFilter*/, QDir::SortFlags sort /*= QDir::NoSort*/) const
{
    return m_HttpFileInfoList;
}

httpfileinfolist httpdir::entryInfoList(const QStringList &nameFilters, QDir::Filters filters /*= QDir::NoFilter*/,
                            QDir::SortFlags sort /*= QDir::NoSort*/) const
{
    return m_HttpFileInfoList;
}

QNetworkReply::NetworkError httpdir::error() const
{
    return m_Error;
}

QString httpdir::errorString() const
{
    return m_strErrorString;
}

void httpdir::setUser(const QString & userName)
{
    m_strUserName = userName;
}

void httpdir::setPassword(const QString & password)
{
    m_strPassword = password;
}

void httpdir::setProxy(const QNetworkProxy * networkProxy)
{
    m_pNetworkProxy = networkProxy;
}
