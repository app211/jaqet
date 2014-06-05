#include "filenamescanner.h"
#include <QDebug>

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

Scanner::AnalysisResult FileNameScanner::analyze( const QFileInfo& fi ) const
{

    // 1. extract base name of the file
    // TODO: analyze the path, too. In case we have something like: "foobar - Season 2/02x12 - blabla.avi"
    //   QFileInfo fi( path );
    //    const QString dir = fi.absolutePath();
    QString name = fi.completeBaseName();

    qDebug() << name;

    // 2. run the base name through all regular expressions we have
    for ( int i = 0; i < m_filenameRegExps.count(); ++i ) {
        QRegExp exp = m_filenameRegExps[i];
        if ( exp.exactMatch( name ) ) {
            AnalysisResult result;

            qDebug() << "Regexp matched:"<<exp;
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

    return AnalysisResult();
}
