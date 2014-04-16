// -------------------------------------------------------------------------------
//  Project:		Qt Webbased File Handling
//
//  File:           httpfileinfo.cpp
//
// (c) 2012 - 2013 by Oblivion Software/Norbert Schlia
// This source code is licensed under The Code Project Open License (CPOL),
// see http://www.codeproject.com/info/cpol10.aspx
// -------------------------------------------------------------------------------
//
#include "httpfileinfo.h"

#include <QRegExp>
#include <QDebug>

httpfileinfo::httpfileinfo()
    : m_i64FileSize(0)
{
    m_MonthNames["Jan"] = "01";
    m_MonthNames["Feb"] = "02";
    m_MonthNames["Mar"] = "03";
    m_MonthNames["Apr"] = "04";
    m_MonthNames["May"] = "05";
    m_MonthNames["Jun"] = "06";
    m_MonthNames["Jul"] = "07";
    m_MonthNames["Aug"] = "08";
    m_MonthNames["Sep"] = "09";
    m_MonthNames["Oct"] = "10";
    m_MonthNames["Nov"] = "11";
    m_MonthNames["Dec"] = "12";
}

//httpfileinfo::httpfileinfo(const QString &file)
// : m_i64FileSize(0)
//{
//}

//httpfileinfo::httpfileinfo(const httpdir &dir, const QString &file)
//{
//}

httpfileinfo::httpfileinfo(const httpfileinfo &fileinfo)
{
    *this = fileinfo;
}

httpfileinfo::~httpfileinfo()
{
}

httpfileinfo & httpfileinfo::operator=(const httpfileinfo & fileinfo)
{
    if (this != &fileinfo)  // or if (*this != io)
    {
        m_Url           = fileinfo.m_Url;
        m_FileTime      = fileinfo.m_FileTime;
        m_strFileTime   = fileinfo.m_strFileTime;
        m_i64FileSize   = fileinfo.m_i64FileSize;
        m_strFileSize   = fileinfo.m_strFileSize;
    }

    return *this;
}

bool httpfileinfo::matchLine(const QUrl & baseUrl, const QString & strLine)
{
//    QRegExp rx("<a href=\"([^?^\"]*)\">([^\"]*)</a>.*([1-9]|[012][0-9]|3[01])-([A-Z][A-Z][A-Z])-(\\d{4}).*([01][0-9]|2[0-4]):([0-5][0-9]).*(\\d{1,}[\\ KMGT]|[0-9]*\\.[0-9]*[\\ KMGT]|-)", Qt::CaseInsensitive);
    QRegExp rx("<a href=\"([^?^\"]*)\">([^\"]*)</a>.*([1-9]|[012][0-9]|3[01])-([A-Z][A-Z][A-Z])-(\\d{4}).*([01][0-9]|2[0-4]):([0-5][0-9])\\ *(\\d{1,}[\\ KMGT]|[0-9]*\\.[0-9]*[\\ KMGT]|-)", Qt::CaseInsensitive);

    if (rx.indexIn(strLine, 0) == -1)
    {
        return false;   // No match
    }

    QString strFileName(rx.cap(1));

    // Url
    m_Url.setUrl(baseUrl.toString() + strFileName);

    // Clear text desc
    //m_str2 = rx.cap(2);
    // Date/Time
    //m_strFileTime = rx.cap(5) + "-" + m_MonthNames[rx.cap(4)] + "-" + rx.cap(3) + "T" + rx.cap(6) + ":" + rx.cap(7) + ":00+01:00";
    m_strFileTime = rx.cap(5) + "-" + m_MonthNames[rx.cap(4)] + "-" + rx.cap(3) + "T" + rx.cap(6) + ":" + rx.cap(7) + ":00";
    m_FileTime = QDateTime::fromString(m_strFileTime, Qt::ISODate);
    // Size
    m_strFileSize = rx.cap(8);
    QString strBuffer(m_strFileSize);
    double dblSize = 0;

    if (rx.cap(8).right(1) == "K")
    {
        strBuffer.chop(1);
        dblSize = strBuffer.toDouble();
        m_i64FileSize = dblSize * 1024;
    }
    else if (rx.cap(8).right(1) == "M")
    {
        strBuffer.chop(1);
        dblSize = strBuffer.toDouble();
        m_i64FileSize = dblSize * 1024 * 1024;
    }
    else if (rx.cap(8).right(1) == "G")
    {
        strBuffer.chop(1);
        dblSize = strBuffer.toDouble();
        m_i64FileSize = dblSize * 1024 * 1024 * 1024;
    }
    else if (rx.cap(8).right(1) == "T")
    {
        strBuffer.chop(1);
        dblSize = strBuffer.toDouble();
        m_i64FileSize = dblSize * 1024 * 1024 * 1024 * 1024;
    }
    else
    {
        dblSize = strBuffer.toDouble();
        m_i64FileSize = dblSize;
    }

    //qDebug() << m_i64FileSize << dblSize << strBuffer << rx.cap(8);

    return true;
}

QUrl httpfileinfo::url() const
{
    return m_Url;
}

QString httpfileinfo::filePath() const
{
    return m_Url.toString();
}

//QString absoluteFilePath() const;
//QString canonicalFilePath() const;
//QString fileName() const;
//QString baseName() const;
//QString completeBaseName() const;
//QString suffix() const;
//QString bundleName() const;
//QString completeSuffix() const;

//QString path() const;
//QString absolutePath() const;
//QString canonicalPath() const;

bool httpfileinfo::isFile() const
{
    return !isDir();
}

bool httpfileinfo::isDir() const
{
    return (m_Url.toString().right(1) == "/");
}

qint64 httpfileinfo::size() const
{
    return m_i64FileSize;
}

QString httpfileinfo::orgSize() const
{
    return m_strFileSize;
}

QDateTime httpfileinfo::created() const
{
    return m_FileTime;
}

QString httpfileinfo::orgCreated() const
{
    return m_strFileTime;
}

//QUrl        m_Url;
//QDateTime   m_FileTime;
//qint64      m_Size;

