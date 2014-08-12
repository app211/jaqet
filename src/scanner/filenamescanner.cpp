#include "filenamescanner.h"
#include <QDebug>
#include <QRegularExpression>

FileNameScanner::FileNameScanner()
{

    // Regular expressions to parse file based on the ones from tvnamer.py
    // Copyright dbr/Ben
    // Regex's to parse filenames with. Must have 3 groups, seriesname, season number
    // and episode number. Use (?: optional) non-capturing groups if you need others.

    // foo_[s01]_[e01]
    m_filenameRegExps.append(
                QRegExp( QLatin1String( "(.+)[ \\._\\-]\\[s([0-9]+)\\]_\\[e([0-9]+)\\]?[^\\\\/]*" ),
                         Qt::CaseInsensitive, QRegExp::RegExp2 ) );

    // foo.1x09*
    m_filenameRegExps.append(
                QRegExp( QLatin1String( "(.+)[ \\._\\-]\\[?([0-9]+)x([0-9]+)[^\\d]?[^\\\\/]*" ),
                         Qt::CaseInsensitive, QRegExp::RegExp2 ) );

    // foo.s01.e01, foo.s01_e01
    m_filenameRegExps.append(
                QRegExp( QLatin1String( "(.+)[ \\._\\-]s([0-9]+)[\\._\\- ]?e([0-9]+)[^\\\\/]*" ),
                         Qt::CaseInsensitive, QRegExp::RegExp2 ) );

    // foo.103* (the strange part at the end is used to 1. prevent another digit and 2. allow the name to end)
    /*   m_filenameRegExps.append(
        QRegExp( QLatin1String( "(.+)[ \\._\\-]([0-9]{1})([0-9]{2})(?:[^\\d][^\\\\/]*)?" ),
                 Qt::CaseInsensitive, QRegExp::RegExp2 ) );
*/
    // foo.0103* (the strange part at the end is used to 1. prevent another digit and 2. allow the name to end)
    /*  m_filenameRegExps.append(
        QRegExp( QLatin1String( "(.+)[ \\._\\-]([0-9]{2})([0-9]{2,3})(?:[^\\d][^\\\\/]*)?" ),
                 Qt::CaseInsensitive, QRegExp::RegExp2 ) );
*/
    // foo Season 01 Episode 02...
    m_filenameRegExps.append(
                QRegExp( QLatin1String( "(.+)[ \\._\\-]season[ \\._\\-]([0-9]+)[ \\._\\-]episode[ \\._\\-]([0-9]+)[ \\._\\-]?.*" ),
                         Qt::CaseInsensitive, QRegExp::RegExp2 ) );

    // foo Book 01 Chapter 02...
    m_filenameRegExps.append(
                QRegExp( QLatin1String( "(.+)[ \\._\\-]*book[ \\._\\-]*([0-9]+).*[ \\._\\-]*chapter[ \\._\\-]*([0-9]+)[ \\._\\-]?.*" ),
                         Qt::CaseInsensitive, QRegExp::RegExp2 ) );
}

Scanner::AnalysisResult FileNameScanner::analyze( const QFileInfo& fi) const
{

    // 1. extract base name of the file
    // TODO: analyze the path, too. In case we have something like: "foobar - Season 2/02x12 - blabla.avi"
    //   QFileInfo fi( path );
    //    const QString dir = fi.absolutePath();
    QString name = fi.completeBaseName();

    qDebug() << name;

    int year=-1;
    QRegExp regex(REGEXYEAR);
    if (name.length() > 4) //movie 2012
    {
        if ( regex.indexIn( name ) >=0) {
            year=regex.capturedTexts().at(0).toInt();
        }
    }

    // 2. run the base name through all regular expressions we have
    for ( int i = 0; i < m_filenameRegExps.count(); ++i ) {
        QRegExp exp = m_filenameRegExps[i];
        if ( exp.exactMatch( name ) ) {
            AnalysisResult result;

            qDebug() << "Regexp matched:"<<exp;
            result.mediaType.year=year;
            result.mediaType.name = exp.cap( 1 ).simplified();
            result.mediaType.season = exp.cap( 2 ).toInt();
            result.mediaType.episode = exp.cap( 3 ).toInt();

            // 3. clean up tv show name
            result.mediaType.name.replace( '.', ' ' );
            result.mediaType.name.replace( '_', ' ' );
            if ( result.mediaType.name.endsWith( '-' ) )
                result.mediaType.name.truncate( result.mediaType.name.length()-1 );
            result.mediaType.name = result.mediaType.name.simplified();

            return result;
        }
    }

    return AnalysisResult(year);
}

QStringList FileNameScanner::REGEXBLACKLIST;

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
const QStringList FileNameScanner::REGEXSERIES = QStringList()
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
const QString FileNameScanner::REGEXSEPARATORS = "[_]";

/* REGEXSPECIAL
 * Used to remove special characters
 * \W Matches a non-word character.
 * Thank you Giuseppe Calà
 */
const QString FileNameScanner::REGEXSPECIAL = "[^\\w\\s-]";

/* REGEXBRACKETS
 * \\(               Include opening bracket
 * [^\\(]            Start match
 * *\\)              Match everyting until closing bracket
 */
// const QString FileParser::REGEXBRACKETS = "\\([^\\(]*\\)|\\[([^]]+)\\]";
const QString FileNameScanner::REGEXBRACKETS = "\\([^\\(]*\\)|\\[([^]]+)\\]|\\{([^}]+)\\}";

/* REGEXYEAR
 * \(19|20) number starting with 19 OR 20
 * \d{2} 2 numbers [0-9]
 */
const QString FileNameScanner::REGEXYEAR = "(19|20)\\d{2}";

QStringList FileNameScanner::readBlacklist()
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



QString FileNameScanner::baseName(const QFileInfo &fileInfo)
{
    //remove file extension
    QString clean = fileInfo.completeBaseName().simplified();

    //remove separator characters
    QRegExp regex(REGEXSEPARATORS);
    clean.replace(regex, " ");

    return clean.simplified();
}

QString FileNameScanner::cleanName(const QString &name)
{
    QString clean = name.simplified();

    qDebug() << "Clean" << name;

    //Ignore all information between brackets.
    QRegExp regex(REGEXBRACKETS);
    clean.replace(regex, " ");
    clean = clean.trimmed();

    qDebug() << "Clean" << clean;

    // Remove the series detection part from the name.
    QStringList::const_iterator constIterator;
    for (constIterator = REGEXSERIES.constBegin(); constIterator != REGEXSERIES.constEnd(); ++constIterator) {
        qDebug() << "Clean" << *constIterator;
        regex.setPattern(*constIterator);
        clean.remove(regex);
    }

    // Remove the year from the name.
    if (clean.length() > 4) //movie 2012
    {

        regex.setPattern(REGEXYEAR);
        clean.remove(regex);
    }

    return clean.simplified();
}

QString FileNameScanner::filterBlacklist(const QString &name)
{
    QString clean = name;
    qDebug() << "TOCLEAN :" <<clean;

    // Remove double char
    clean.replace("..", ".");
    clean.replace("  ", " ");
    clean.replace("__", "_");
   // qDebug() << "TOCLEAN :" <<clean;

    //Remove special characters.
    QRegExp regex(REGEXSPECIAL);
   // clean.remove(regex);
    qDebug() << "TOCLEAN :" <<clean;

    clean = clean.toUpper();
    qDebug() << "TOCLEAN :" <<clean;

    if (REGEXBLACKLIST.isEmpty()){
        REGEXBLACKLIST=readBlacklist();
    }

    //Remove blacklisted regexp
    foreach(const QString & pattern, REGEXBLACKLIST) {

        QRegularExpression reg(pattern);
        qDebug() << "regex:" <<pattern;
        clean.replace(reg, " ");
        qDebug() << clean;
    }

    clean.replace(".", " ");
    clean.replace("_", " ");

    return clean.simplified();
}

QString FileNameScanner::getFilteredName(const QFileInfo &fileInfo) const {
    QString baseName = FileNameScanner::baseName(fileInfo);
    QString name =FileNameScanner::cleanName(baseName);
   return  filterBlacklist(name);
}


