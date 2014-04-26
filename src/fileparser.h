#ifndef FILEPARSER_H
#define FILEPARSER_H

class QString;
class QStringList;
class QFileInfo;

class FileParser
{
public:
    FileParser();

    static bool isSeries(const QString &name, QString &season, QString& episode);
    static QString cleanName(const QString& name);
    static QString filterBlacklist(const QString& name);
    static QString baseName(const QFileInfo &fileInfo);

private:

    static const QStringList REGEXSERIES;

    static const QString REGEXSEPARATORS;

    static const QString REGEXSPECIAL;

    static QStringList REGEXBLACKLIST;

    static const QString REGEXBRACKETS;

      static const QString REGEXYEAR;

    static QStringList readBlacklist();

};

#endif // FILEPARSER_H
