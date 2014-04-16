// -------------------------------------------------------------------------------
//  Project:		Qt Webbased File Handling
//
//  File:           httpdir.h
//
// (c) 2012 - 2013 by Oblivion Software/Norbert Schlia
// This source code is licensed under The Code Project Open License (CPOL),
// see http://www.codeproject.com/info/cpol10.aspx
// -------------------------------------------------------------------------------
//
#pragma once

#ifndef QHTTPDIR_H
#define QHTTPDIR_H

#include "httpfileinfo.h"

#include <QDir>
#include <QNetworkReply>

class httpdir
{
public:
    httpdir(const httpdir & strUrl);
    httpdir(const QUrl & url);
    httpdir(const QString &path = QString());
    httpdir(const QString &path, const QString &nameFilter,
         QDir::SortFlags sort = QDir::SortFlags(QDir::Name | QDir::IgnoreCase), QDir::Filters filter = QDir::AllEntries);
    ~httpdir();

    httpdir &operator=(const httpdir &);
    httpdir &operator=(const QString & path);

//    inline void swap(httpdir &other)
//    { qSwap(d_ptr, other.d_ptr); }

    void setUrl(const QString & path);
    void setUrl(const QUrl & url);
    QUrl url() const;
    QString absolutePath() const;
    QString canonicalPath() const;

    QString dirName() const;
    QString filePath(const QString &fileName) const;
    QString absoluteFilePath(const QString &fileName) const;
    QString relativeFilePath(const QString &fileName) const;

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filter);
    QDir::SortFlags sorting() const;
    void setSorting(QDir::SortFlags sort);

    uint count() const;
    QString operator[](int) const;

    static QStringList nameFiltersFromString(const QString &nameFilter);

    QStringList entryList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    QStringList entryList(const QStringList &nameFilters, QDir::Filters filters = QDir::NoFilter,
                          QDir::SortFlags sort = QDir::NoSort) const;

    httpfileinfolist entryInfoList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    httpfileinfolist entryInfoList(const QStringList &nameFilters, QDir::Filters filters = QDir::NoFilter,
                                QDir::SortFlags sort = QDir::NoSort) const;

//    bool exists() const;

    static bool isRelativePath(const QString &path);
    inline static bool isAbsolutePath(const QString &path) { return !isRelativePath(path); }
    bool isRelative() const;
    inline bool isAbsolute() const { return !isRelative(); }
    bool makeAbsolute();

    bool operator==(const httpdir &dir) const;
    inline bool operator!=(const httpdir &dir) const {  return !operator==(dir); }

//    bool exists(const QString &name) const;

    static QFileInfoList drives();

    static QChar separator();

#ifndef QT_NO_REGEXP
    static bool match(const QStringList &filters, const QString &fileName);
    static bool match(const QString &filter, const QString &fileName);
#endif

    static QString cleanPath(const QString &path);
    void refresh();

    QNetworkReply::NetworkError error() const;
    QString                     errorString() const;

    void                        setUser(const QString & userName);
    void                        setPassword(const QString & password);
    void                        setProxy(const QNetworkProxy * networkProxy);

protected:
    QUrl                        m_Url;
    httpfileinfolist           m_HttpFileInfoList;
    QString                     m_strErrorString;
    QNetworkReply::NetworkError m_Error;
    QString                     m_strUserName;
    QString                     m_strPassword;
    const QNetworkProxy *       m_pNetworkProxy;
};

#endif // QHTTPDIR_H
