#include "fileparser.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo>

QStringList FileParser::REGEXBLACKLIST;

/* REGEXSERIES
 * [0-9]+                 Can be one or two numbers.
 * (19|20)\\d{2}          year
 * 0[1-9] | 1[012]        Number from 01 to 09 OR 10 to 12 (month)
 * (0[1-9]|[12]\\d)|3[01] Number 01 - 31 (day)
 * (?i)                   Ignore case
 * [\\[|\\(]              '[' or '['
 * [a-zA-Z0-9\\s]{8}      8 alphanumeric chars
 * \s Matches a whitespace character (QChar::isSpace()).
 */
const QStringList FileParser::REGEXSERIES = QStringList()
        << "[sS]([0-9]+)\\s*[eE]([0-9]+)(.*)" // S00E00
        << "\\s[Ee][Pp]\\s?([0-9]+)(.*)" // Ep.00
        << "\\s([0-9]+)x[0-9]{2}\\s" // blah.100
        << "\\s(?i)p(?:ar)?t\\s" //Part Pt.
        << "(19|20)\\d{2}\\s((0[1-9])|(1[012]))\\s((0[1-9]|[12]\\d)|3[01])"  // yyyy-mm-dd
        << "((0[1-9]|[12]\\d)|3[01])\\s((0[1-9])|(1[012]))\\s(19|20)\\d{2}"  // dd-mm-yyyy
        << ("[\\[|\\(][a-zA-Z0-9]{8}[\\]|\\)]"); //[abCD5678] or (abCD5678)

/* REGEXSEPARATORS
 * Remove underscores, etc. from filenames
 * Dots are removed later, they can be part of a title
 */
const QString FileParser::REGEXSEPARATORS = "[_]";

/* REGEXSPECIAL
 * Used to remove special characters
 * \W Matches a non-word character.
 * Thank you Giuseppe Calà
 */
const QString FileParser::REGEXSPECIAL = "[^\\w\\s]";

/* REGEXBRACKETS
 * \\(               Include opening bracket
 * [^\\(]            Start match
 * *\\)              Match everyting until closing bracket
 */
// const QString FileParser::REGEXBRACKETS = "\\([^\\(]*\\)|\\[([^]]+)\\]";
const QString FileParser::REGEXBRACKETS = "\\([^\\(]*\\)|\\[([^]]+)\\]|\\{([^}]+)\\}";

/* REGEXYEAR
 * \(19|20) number starting with 19 OR 20
 * \d{2} 2 numbers [0-9]
 */
const QString FileParser::REGEXYEAR = "(19|20)\\d{2}";

QStringList FileParser::readBlacklist()
{
    QStringList list;
    QString tempWord;

    //Open the file
    QFile file(":/tizzbird/data/query-blacklist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //Something went wrong: return empty list
        qDebug() << "blacklist file not found!";
        return list;
    }

    //Read every line and store it in the list
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        tempWord = stream.readLine();
        if (!tempWord.startsWith('#') && !tempWord.isEmpty()) {
            list << tempWord.trimmed();
        }
    }
    return list;
}



bool FileParser::isSeries(const QString &name, QString &season, QString& episode)
{
    // Check if the file is a series.
    QRegExp regex;

    regex.setPattern("[sS]([0-9]+)\\s*[eE]([0-9]+)(.*)"); // S00E00
    if (regex.lastIndexIn(name) != -1) {
        QStringList list = regex.capturedTexts();
        if (list.size()>2){
            season=list.at(1);
            episode=list.at(2);
            return true;
        }
    }


    /* QStringList::const_iterator constIterator;
    for (constIterator = REGEXSERIES.constBegin(); constIterator != REGEXSERIES.constEnd(); ++constIterator) {
        regex.setPattern(*constIterator);
        if (regex.lastIndexIn(name) != -1) {
            if (!regex.isEmpty()) {
                return true;
            }
        }
    }*/
    return false;
}

QString FileParser::baseName(const QFileInfo &fileInfo)
{
    //remove file extension
    QString clean = fileInfo.completeBaseName().simplified();

    //remove separator characters
    QRegExp regex(REGEXSEPARATORS);
    clean.replace(regex, " ");

    return clean.simplified();
}

QString FileParser::cleanName(const QString &name)
{
    QString clean = name.simplified();

    //Ignore all information between brackets.
    QRegExp regex(REGEXBRACKETS);
    clean.replace(regex, " ");
    clean = clean.trimmed();

    // Remove the series detection part from the name.
    QStringList::const_iterator constIterator;
    for (constIterator = REGEXSERIES.constBegin(); constIterator != REGEXSERIES.constEnd(); ++constIterator) {
        regex.setPattern(*constIterator);
        clean.remove(regex);
    }

    // Remove the year from the name.
    if (clean.length() > 4) //movie 2012
        regex.setPattern(REGEXYEAR);
    clean.remove(regex);

    return clean.simplified();
}

QString FileParser::filterBlacklist(const QString &name)
{
    QString clean = name;

    // Remove dots
    clean.replace(".", " ");

    //Remove special characters.
    QRegExp regex(REGEXSPECIAL);
    clean.remove(regex);

    clean = clean.toUpper();

    if (REGEXBLACKLIST.isEmpty()){
        REGEXBLACKLIST=readBlacklist();
    }

    //Remove blacklisted regexp
    foreach(const QString & pattern, REGEXBLACKLIST) {

        QRegularExpression reg(pattern);
        clean.replace(reg, " ");
    }

    return clean.simplified();
}


