#include "thetvdbscraper.h"
#include <QUrl>
#include <QDebug>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QPixmap>
#include <QByteArray>
#include <QNetworkReply>
#include <QBuffer>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "../utils.h"
#include "../promise.h"

// Cf. http://thetvdb.com/wiki/index.php?title=Programmers_API
static QList<QString> m_xmlmirrors;
static QList<QString> m_bannermirrors;
static QList<QString> m_zipmirrors;
static bool retrieveMirror = true;

const QString TheTVDBScraper::API_KEY="C526A71D6E158EF0";

TheTVDBScraper::TheTVDBScraper(QObject *parent)
    : Scraper(parent)
{
}

QString TheTVDBScraper::getXMLURL() const {
    if (m_xmlmirrors.size()>0){
        return m_xmlmirrors[Utils::randInt(0,m_xmlmirrors.size()-1)];
    }

    return "http://thetvdb.com";
}

QString TheTVDBScraper::getZIPURL() const {
    if (m_zipmirrors.size()>0){
        return m_zipmirrors[Utils::randInt(0,m_zipmirrors.size()-1)];
    }

    return "http://thetvdb.com";
}

QString TheTVDBScraper::getBannerURL() const {
    if (m_bannermirrors.size()>0){
        return m_bannermirrors[Utils::randInt(0,m_bannermirrors.size()-1)];
    }

    return "http://thetvdb.com";
}

QString TheTVDBScraper::createURL(const QString& command, const QMap<QString, QString>& params) const {

    QString queryURL=getXMLURL().append("/api/").append(command);

    if (params.size()==0){
        qDebug() << queryURL;

        return queryURL;
    }

    QString mapParams;
    bool firstParameter=true;

    foreach( QString key, params.keys() )
    {
        if (!firstParameter){
            mapParams.append('&');
        }

        firstParameter=false;
        mapParams.append(QUrl::toPercentEncoding(key)).append('=').append(params.value( QUrl::toPercentEncoding(key) ));
    }

    QString fullQuery= queryURL.append('?').append(mapParams);

    qDebug() << fullQuery;

    return fullQuery;
}


void TheTVDBScraper::parseSeriesList( QNetworkAccessManager* manager, const QString& toSearch, const QByteArray& data, const QString& langage)
{

    QMap<QString,QList<QPair<QString,ShowPtr>>> series;

    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() ) {
        while ( xml.readNextStartElement() &&
                xml.name() == QLatin1String( "Series" ) ) {
            int id = 0;
            QString name, overview,langage;
            while ( xml.readNextStartElement() ) {
                if ( xml.name() == QLatin1String( "seriesid" ) ) {
                    id = xml.readElementText().toInt();
                }
                else if ( xml.name() == QLatin1String( "SeriesName" ) ) {
                    name = xml.readElementText();
                }
                else if ( xml.name() == QLatin1String( "Overview" ) ) {
                    overview = xml.readElementText();
                }
                else if ( xml.name() == QLatin1String( "language")){
                    langage=xml.readElementText();
                }
                else {
                    // skip over this tag
                    xml.skipCurrentElement();
                }
            }
            if ( id > 0 && !name.isEmpty() && !langage.isEmpty() ) {
                qDebug() << "found series Item:" << langage << id << name;
                ShowPtr show(new Show());
                show->code=QString::number(id);
                show->title=name;

                if (!series.contains(show->code)){
                    series.insert(show->code,QList<QPair<QString,ShowPtr>>());
                }

                QPair<QString,ShowPtr> lang_show;
                lang_show.first=langage;
                lang_show.second=show;
                series[show->code].append(lang_show);
            }
            else {
                qDebug() << "invalid Item:" << id << name;
                //   return false;
            }
        }
    }

    ShowPtrList shows;
    for(const QString& serieid: series.keys()){
        bool found=false;
        for (QPair<QString,ShowPtr> lang_show: series[serieid]) {
            if (lang_show.first=="fr"){
                shows.append(lang_show.second);
                found=true;
                break;
            }
        }

        if (!found && series[serieid].size()>0){
            shows.append(series[serieid].at(0).second);
        }
    }

    emit found(shows);
}

void TheTVDBScraper::parseMirrorList( const QByteArray& data)
{
    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() ) {
        while ( xml.readNextStartElement() &&
                xml.name() == QLatin1String( "Mirror" ) ) {
            int id = 0;
            int typemask = 0;
            QString  mirrorpath;
            while ( xml.readNextStartElement() ) {
                if ( xml.name() == QLatin1String( "id" ) ) {
                    id = xml.readElementText().toInt();
                }
                else if ( xml.name() == QLatin1String( "mirrorpath" ) ) {
                    mirrorpath = xml.readElementText();
                }
                else if ( xml.name() == QLatin1String( "typemask" ) ) {
                    typemask = xml.readElementText().toInt();
                }
                else {
                    // skip over this tag
                    xml.skipCurrentElement();
                }
            }

            if (id> 0 && typemask>0 && !mirrorpath.isEmpty()){
                // Cf. http://thetvdb.com/wiki/index.php?title=API:mirrors.xml
                if ((typemask&1)!=0){
                    // 1 xml files
                    m_xmlmirrors.append(mirrorpath);
                }
                if ((typemask&2)!=0){
                    //2 banner files
                    m_bannermirrors.append(mirrorpath);
                }
                if ((typemask&4)!=0){
                    //4 zip files
                    m_zipmirrors.append(mirrorpath);
                }
            }
        }
    }
}


void TheTVDBScraper::internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const {
    emit scraperError("Unsupported Operation");
}

bool parseInfo(const QByteArray &data, SearchEpisodeInfo& result, const int season, const int episode){
    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() && xml.name()==QLatin1String("Data")){
        while ( xml.readNextStartElement()) {
            if (xml.name() == QLatin1String( "Episode" ) ) {
                int episodeNumber=-1;
                int seasonNumber=-1;
                QString overview;
                while ( xml.readNextStartElement()) {
                    if ( xml.name() == QLatin1String( "EpisodeNumber" ) ) {
                        episodeNumber=xml.readElementText().toInt();
                    } else if ( xml.name() == QLatin1String( "SeasonNumber" ) ) {
                        seasonNumber=xml.readElementText().toInt();
                    }else if ( xml.name() == QLatin1String( "Overview" ) ) {
                        overview=xml.readElementText();
                    }
                    else {
                        xml.skipCurrentElement();
                    }
                }

                if (episodeNumber==episode&&season==seasonNumber){
                    result.synopsis=overview;
                    qDebug() << "BINGO!!!" << overview;
                    return true;
                }
            } else  {
                xml.skipCurrentElement();
            }
        }
    }

    return false;
}

bool parseBanner(const QByteArray &data, SearchEpisodeInfo& result){
    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() && xml.name()==QLatin1String("Banners")){
        while ( xml.readNextStartElement()) {
            if (xml.name() == QLatin1String( "Banner" ) ) {
                QString bannerPath;
                QString bannerType;
                 while ( xml.readNextStartElement() ) {
                    if ( xml.name() == QLatin1String( "id" ) ) {
                        xml.skipCurrentElement();
                        //       qDebug() << "id " << xml.readElementText().toInt();
                    } else if ( xml.name() == QLatin1String( "BannerPath" ) ) {
                        bannerPath=  xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "BannerType" ) ) {
                        bannerType= xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "BannerType2" ) ) {
                        xml.skipCurrentElement();
                        //      qDebug() << "BannerType2 " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "Colors" ) ) {
                        xml.skipCurrentElement();
                        //       qDebug() << "Colors " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "Language" ) ) {
                        xml.skipCurrentElement();
                        //      qDebug() << "Language " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "RatingCount" ) ) {
                        xml.skipCurrentElement();
                        //     qDebug() << "RatingCount " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "SeriesName" ) ) {
                        xml.skipCurrentElement();
                        //    qDebug() << "SeriesName " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "ThumbnailPath" ) ) {
                        xml.skipCurrentElement();
                        //    qDebug() << "ThumbnailPath " << xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "VignettePath" ) ) {
                        xml.skipCurrentElement();
                        //    qDebug() << "VignettePath " << xml.readElementText();
                    } else {
                        xml.skipCurrentElement();
                    }
                }

                if (!bannerPath.isEmpty() && bannerType==QLatin1String( "series" )){
                    result.bannersHref.append(bannerPath);
                }
            } else {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

bool parseActors(const QByteArray &data, SearchEpisodeInfo& result){
    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() && xml.name()==QLatin1String("Actors")){
        QStringList actors[4];
        while ( xml.readNextStartElement()) {
            if (xml.name() == QLatin1String( "Actor" ) ) {
                QString actorName;
                QString actorSortOrder;
                 while ( xml.readNextStartElement() ) {
                    if ( xml.name() == QLatin1String( "Name" ) ) {
                        actorName=  xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "SortOrder" ) ) {
                        actorSortOrder= xml.readElementText();
                    } else {
                        xml.skipCurrentElement();
                    }
                }

                 if (!actorName.isEmpty() && !actorSortOrder.isEmpty()){
                     bool bOk;
                     int order = actorSortOrder.toInt(&bOk);
                     if (bOk && order>=0 && order<=3){
                         actors[order].append(actorName);
                     }
                 }
            } else {
                return false;
            }


        }

        result.actors << actors[0];
        result.actors << actors[1];
        result.actors << actors[2];
        result.actors << actors[3];

        return true;
    } else {
        return false;
    }
}


void TheTVDBScraper::internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const {

    QString lang="fr";
    QString command=QString::fromLatin1( "%1/series/%2/all/%3.zip" ).arg(API_KEY).arg( showCode ).arg( lang ) ;

    QMap<QString,QString> params;
    QString url=createURL(command,params);
    Promise* promise=Promise::loadAsync(*manager,url,false);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            QByteArray data= promise->reply->readAll();
            SearchEpisodeInfo result;
            QBuffer buffer(&data);
            QuaZip replyZip(&buffer);
            replyZip.open(QuaZip::mdUnzip);

            for(bool f=replyZip.goToFirstFile(); f; f=replyZip.goToNextFile()) {
                QuaZipFile zFile( &replyZip);

                zFile.open( QIODevice::ReadOnly );

                QString currentFileName=replyZip.getCurrentFileName();
                if (currentFileName.endsWith("banners.xml",Qt::CaseInsensitive)){
                    parseBanner( zFile.readAll(), result);
                }else if (currentFileName.endsWith("actors.xml",Qt::CaseInsensitive)){
                    parseActors( zFile.readAll(), result);
                } else if (currentFileName.endsWith(QString("%1.xml").arg(lang),Qt::CaseInsensitive)){
                    parseInfo( zFile.readAll(), result,season, episode);
                }

                zFile.close();

            }

            replyZip.close();

            emit found(this,result);
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}


void TheTVDBScraper::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const {
    emit scraperError("Unsupported Operation");
}

void TheTVDBScraper::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const{
    if  (retrieveMirror){
        retrieveMirror= false;
        QString url = QString("http://thetvdb.com/api/%1/mirrors.xml").arg(API_KEY);
        Promise* promise=Promise::loadAsync(*manager,url,false);
        QObject::connect(promise, &Promise::completed, [=]()
        {
            QByteArray data= promise->reply->readAll();
            if (promise->reply->error() ==QNetworkReply::NoError){
                parseMirrorList(data);
                internalSearchTV(manager,toSearch,language);
            } else {
                emit scraperError(tr("Unable to retrieve 'mirrors.xml'"));
            }
        });
    }
    else {
        QMap<QString,QString> params;
        params["language"]=QUrl::toPercentEncoding(language);
        params["seriesname"]=QUrl::toPercentEncoding(toSearch);
        QString url=createURL("GetSeries.php",params);
        Promise* promise=Promise::loadAsync(*manager,url,false);
        QObject::connect(promise, &Promise::completed, [=]()
        {
            QByteArray data= promise->reply->readAll();
            if (promise->reply->error() ==QNetworkReply::NoError){
                parseSeriesList(manager,toSearch, data,language);
            }else {
                emit scraperError(promise->reply->errorString());
            }
        });
    }
}

QString TheTVDBScraper::getBestImageUrl(const QString& url, const QSize& size, ImageType imageType) const {
    if (imageType==ImageType::BANNER){
        return getBannerURL().append("/banners/").append(url);
    }
}

const uchar TheTVDBScraper::icon_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff,
    0x61, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
    0x88, 0x00, 0x00, 0x01, 0xe4, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0x8d, 0x92, 0x3f, 0x68, 0x13,
    0x51, 0x1c, 0xc7, 0x3f, 0x8d, 0xd5, 0xb6, 0x90, 0xa4, 0x05, 0x9d, 0x72, 0x8d, 0xcd, 0xa0, 0x46,
    0xa3, 0x21, 0x14, 0x12, 0x48, 0x28, 0x92, 0xc3, 0xa5, 0xba, 0x84, 0x0e, 0x59, 0x25, 0x08, 0x2e,
    0x4e, 0xe2, 0xe0, 0xe4, 0x12, 0xb0, 0x83, 0x53, 0x5a, 0x5c, 0x0a, 0x52, 0x4b, 0x40, 0xa7, 0x0a,
    0x72, 0x2a, 0x0e, 0x59, 0xae, 0x29, 0x82, 0x2d, 0x5a, 0x24, 0x0e, 0xa9, 0x71, 0x10, 0xee, 0x34,
    0x77, 0x20, 0x14, 0x4c, 0x9a, 0x98, 0x8a, 0x42, 0x9f, 0x43, 0x7a, 0x4f, 0x53, 0x0e, 0xcf, 0x1f,
    0x3c, 0x78, 0xff, 0x7e, 0x9f, 0xdf, 0xf7, 0xfb, 0x7e, 0x6f, 0x88, 0x70, 0x5c, 0xf0, 0x9f, 0x71,
    0xef, 0xae, 0xc2, 0xe6, 0x9b, 0xef, 0x68, 0x2f, 0x5a, 0x72, 0x6f, 0xd8, 0x2b, 0x49, 0x09, 0x1d,
    0xe5, 0xda, 0xd5, 0x13, 0x00, 0xa8, 0x17, 0x03, 0x44, 0xcf, 0x8c, 0x72, 0x21, 0x36, 0x46, 0x6f,
    0x6f, 0x9f, 0xd2, 0xfd, 0xaf, 0xf8, 0xbc, 0x00, 0x96, 0xfd, 0x8b, 0xd7, 0x9b, 0x5d, 0xae, 0xcc,
    0x06, 0x19, 0x19, 0x19, 0x22, 0x72, 0xf2, 0x18, 0xd3, 0x89, 0x31, 0x9e, 0x6a, 0xdf, 0x00, 0xfa,
    0x80, 0xc8, 0x64, 0x88, 0xea, 0xea, 0x0a, 0xc5, 0x5b, 0x37, 0x5c, 0x21, 0xfa, 0x7a, 0x87, 0xf5,
    0x57, 0x5d, 0xb9, 0x5e, 0x2e, 0xef, 0x60, 0x7c, 0xfe, 0xf9, 0x17, 0x20, 0xac, 0x90, 0x4d, 0x27,
    0x51, 0x33, 0x29, 0x57, 0x80, 0xcf, 0x07, 0xe7, 0xce, 0x8e, 0x72, 0xfb, 0x8e, 0xc5, 0x83, 0x95,
    0x1d, 0x66, 0xd2, 0x7e, 0x79, 0x76, 0x24, 0x72, 0x7e, 0xba, 0xa8, 0x66, 0x52, 0x5c, 0x56, 0x67,
    0x68, 0xef, 0x76, 0x68, 0x7c, 0x32, 0x88, 0x84, 0x15, 0x8c, 0xa6, 0x2d, 0x81, 0xfb, 0xa2, 0xc7,
    0xf3, 0x97, 0x2d, 0xb6, 0x3f, 0xfc, 0x60, 0xeb, 0x5d, 0x8f, 0xbd, 0xde, 0x71, 0x4e, 0x4d, 0x9d,
    0xc6, 0x68, 0xda, 0xb0, 0xf8, 0xf0, 0xb1, 0x70, 0x8b, 0xb9, 0xeb, 0x37, 0x85, 0x10, 0x42, 0xd4,
    0xea, 0x0d, 0x41, 0x38, 0x3e, 0x30, 0x8c, 0xa6, 0x25, 0x84, 0x10, 0x82, 0x70, 0x5c, 0x0c, 0x6b,
    0x15, 0x9d, 0x89, 0x60, 0x80, 0x42, 0x3e, 0x87, 0x69, 0xd9, 0x94, 0x57, 0x9f, 0x01, 0xa0, 0x55,
    0x74, 0x4c, 0xcb, 0x26, 0x11, 0x8b, 0x12, 0x99, 0x0c, 0xf5, 0xab, 0x1d, 0xbc, 0xd7, 0x94, 0x12,
    0xe2, 0xfd, 0xf6, 0xc7, 0xbe, 0xbd, 0xea, 0xc6, 0x5b, 0xca, 0x4f, 0xfa, 0x49, 0xc6, 0x17, 0x9b,
    0xe2, 0xc2, 0x12, 0xc5, 0x85, 0xa5, 0x03, 0xc8, 0x1a, 0x00, 0x73, 0xb3, 0x97, 0xa4, 0x67, 0x67,
    0xee, 0xe4, 0xfc, 0xb3, 0x8d, 0xce, 0x25, 0x37, 0x80, 0x56, 0xd1, 0x01, 0x8f, 0x8f, 0x54, 0xab,
    0x37, 0x30, 0x2d, 0x9b, 0x6c, 0x3a, 0xc9, 0x44, 0x30, 0x00, 0x40, 0x36, 0x9d, 0xc4, 0xb4, 0x6c,
    0x69, 0xc9, 0xf3, 0x23, 0x55, 0x37, 0xb6, 0x64, 0xe5, 0x3f, 0xd5, 0xd7, 0xe4, 0xb9, 0x27, 0xc0,
    0x91, 0xaa, 0x66, 0x52, 0xb2, 0xad, 0x8e, 0x35, 0x69, 0xa1, 0x56, 0x6f, 0x00, 0x48, 0x99, 0x87,
    0x01, 0xed, 0x4e, 0x57, 0x56, 0x37, 0x2d, 0x5b, 0xde, 0x97, 0x0a, 0x5a, 0xbb, 0x1d, 0xda, 0x9d,
    0x2e, 0x89, 0x58, 0x94, 0x72, 0x69, 0x9e, 0x72, 0x69, 0x7e, 0x00, 0xa6, 0x55, 0x74, 0xc6, 0x03,
    0x7e, 0xc6, 0x03, 0xfe, 0x01, 0xf9, 0x03, 0x16, 0x16, 0x97, 0x1f, 0x01, 0x50, 0xc8, 0xe7, 0x28,
    0xe4, 0x73, 0xae, 0x36, 0x0e, 0xcb, 0x07, 0xf8, 0x0d, 0x41, 0xe5, 0xda, 0x32, 0x93, 0x20, 0x23,
    0x88, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82} ;

static QIcon m_icon;

QIcon TheTVDBScraper::getIcon() const {
    if (m_icon.isNull()){
        QPixmap pixmap;
        if (pixmap.loadFromData(TheTVDBScraper::icon_png, sizeof(TheTVDBScraper::icon_png)/sizeof(uchar))){
            m_icon=QIcon(pixmap);
        }
    }

    return m_icon;
}


QString TheTVDBScraper::getName() const {
    return "TVDB";
}
