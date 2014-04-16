// -------------------------------------------------------------------------------
//  Project:		Qt Webbased File Handling
//
//  File:           httpfileinfo.h
//
// (c) 2012 - 2013 by Oblivion Software/Norbert Schlia
// This source code is licensed under The Code Project Open License (CPOL),
// see http://www.codeproject.com/info/cpol10.aspx
// -------------------------------------------------------------------------------
//
#pragma once

#ifndef QHTTPFILE_H
#define QHTTPFILE_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QUrl>

class httpdir;

class httpfileinfo
{
    friend class httpdir;

public:
    httpfileinfo();
    httpfileinfo(const QString &file);
//    httpfileinfo(const QFile &file);
    httpfileinfo(const httpdir &dir, const QString &file);
    httpfileinfo(const httpfileinfo &fileinfo);
    ~httpfileinfo();

    httpfileinfo &operator=(const httpfileinfo &fileinfo);
    bool operator==(const httpfileinfo &fileinfo); // 5.0 - remove me
    bool operator==(const httpfileinfo &fileinfo) const;
    inline bool operator!=(const httpfileinfo &fileinfo) { return !(operator==(fileinfo)); } // 5.0 - remove me
    inline bool operator!=(const httpfileinfo &fileinfo) const { return !(operator==(fileinfo)); }

    void setFile(const QString &file);
//    void setFile(const QFile &file);
    void setFile(const httpdir &dir, const QString &file);
    bool exists() const;
    void refresh();

    QUrl url() const;
    QString filePath() const;
    QString absoluteFilePath() const;
    QString canonicalFilePath() const;
    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString bundleName() const;
    QString completeSuffix() const;

    QString path() const;
    QString absolutePath() const;
    QString canonicalPath() const;
//    httpdir dir() const;
//    httpdir absoluteDir() const;

    bool isReadable() const;

    bool isRelative() const;
    inline bool isAbsolute() const { return !isRelative(); }
    bool makeAbsolute();

    bool isFile() const;
    bool isDir() const;

    qint64 size() const;
    QString orgSize() const;

    QDateTime created() const;
//    QDateTime lastModified() const;
//    QDateTime lastRead() const;
    QString orgCreated() const;

//    void detach();

protected:
    bool    matchLine(const QUrl & baseUrl, const QString & strLine);

protected:
    QHash<QString, QString> m_MonthNames;
    QUrl                    m_Url;
    QDateTime               m_FileTime;
    QString                 m_strFileTime;
    qint64                  m_i64FileSize;
    QString                 m_strFileSize;
};

typedef QList<httpfileinfo> httpfileinfolist;


#endif // QHTTPFILE_H
