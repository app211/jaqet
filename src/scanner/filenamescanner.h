#ifndef FILENAMESCANNER_H
#define FILENAMESCANNER_H

#include <QList>
#include <QRegExp>
#include <QFileInfo>
#include "scanner.h"

class FileNameScanner : public Scanner
{
public:
    FileNameScanner();
    AnalysisResult analyze(const QFileInfo& fi ) const;
    QString getFilteredName(const QFileInfo &fileInfo) const;

private:
    static QString cleanName(const QString& name);
    static QString filterBlacklist(const QString& name);
    static QString baseName(const QFileInfo &fileInfo);

    static const QStringList REGEXSERIES;

    static const QString REGEXSEPARATORS;

    static const QString REGEXSPECIAL;

    static QStringList REGEXBLACKLIST;

    static const QString REGEXBRACKETS;

      static const QString REGEXYEAR;

    static QStringList readBlacklist();
private:
     QList<QRegExp> m_filenameRegExps;
};

#endif // FILENAMESCANNER_H
