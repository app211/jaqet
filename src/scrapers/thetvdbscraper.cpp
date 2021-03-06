#include "thetvdbscraper.h"
#include <QUrl>
#include <QDebug>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QPixmap>
#include <QByteArray>
#include <QNetworkReply>
#include <QBuffer>
#include <QCoreApplication>
#include <QRegularExpression>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "../utils.h"
#include "../promise.h"

// Cf. http://thetvdb.com/wiki/index.php?title=Programmers_API
static QList<QString> m_xmlmirrors;
static QList<QString> m_bannermirrors;
static QList<QString> m_zipmirrors;
static bool retrieveMirror = true;

namespace  {
    const QString API_KEY(QStringLiteral("C526A71D6E158EF0"));
}

TheTVDBScraper::TheTVDBScraper(QObject *parent)
    : Scraper(parent), m_icon(loadIcon())
{
    static const char *genres[] = {
        QT_TR_NOOP("All"),
        QT_TR_NOOP("Action"),
        QT_TR_NOOP("Adventure"),
        QT_TR_NOOP("Animation"),
        QT_TR_NOOP("Children"),
        QT_TR_NOOP("Comedy"),
        QT_TR_NOOP("Crime"),
        QT_TR_NOOP("Documentary"),
        QT_TR_NOOP("Drama"),
        QT_TR_NOOP("Family"),
        QT_TR_NOOP("Fantasy"),
        QT_TR_NOOP("Food"),
        QT_TR_NOOP("Game Show"),
        QT_TR_NOOP("Home and Garden"),
        QT_TR_NOOP("Horror"),
        QT_TR_NOOP("Mini-Series"),
        QT_TR_NOOP("Mystery"),
        QT_TR_NOOP("News"),
        QT_TR_NOOP("Reality"),
        QT_TR_NOOP("Romance"),
        QT_TR_NOOP("Science-Fiction"),
        QT_TR_NOOP("Soap"),
        QT_TR_NOOP("Special Interest"),
        QT_TR_NOOP("Sport"),
        QT_TR_NOOP("Suspense"),
        QT_TR_NOOP("Talk Show"),
        QT_TR_NOOP("Thriller"),
        QT_TR_NOOP("Travel"),
        QT_TR_NOOP("Western")
    };

    for ( const char * genre: genres) {
        genreTranslator[genre]=tr(genre);
    }

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
    Q_UNUSED(manager)
    Q_UNUSED(toSearch)
    Q_UNUSED(langage)

    QMap<QString,QList<QPair<QString,ShowPtr>>> series;

    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() ) {
        while ( xml.readNextStartElement() &&
                xml.name() == QLatin1String( "Series" ) ) {
            int id = 0;
            QString name, overview,langage;
            int productionYear=-1;
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
                }  else if ( xml.name() == QLatin1String( "FirstAired" ) ) {
                    QDate date = QDate::fromString(xml.readElementText(), "yyyy-MM-dd");
                    if (date.isValid() ){
                        productionYear=date.year();
                    }
                }
                else {
                    // skip over this tag
                    xml.skipCurrentElement();
                }
            }
            if ( id > 0 && !name.isEmpty() && !langage.isEmpty() ) {
                ShowPtr show(new Show());
                show->code=QString::number(id);
                show->title=name;
                if (productionYear>=0){
                    show->productionYear=QString::number(productionYear);
                }
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

bool TheTVDBScraper::parseMirrorList( const QByteArray& data)
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

    return true;
}


// Cf. http://thetvdb.com/wiki/index.php?title=API:languages.xml

bool TheTVDBScraper::parseLanguageList( const QByteArray& data)
{

    QXmlStreamReader xml( data );
    if ( xml.readNextStartElement() ) {
        while ( xml.readNextStartElement() &&
                xml.name() == QLatin1String( "Language" ) ) {
            int id = 0;
            QString  name;
            QString  abbreviation;

            while ( xml.readNextStartElement() ) {
                if ( xml.name() == QLatin1String( "id" ) ) {
                    id = xml.readElementText().toInt();
                }
                else if ( xml.name() == QLatin1String( "name" ) ) {
                    name = xml.readElementText();
                }
                else if ( xml.name() == QLatin1String( "abbreviation" ) ) {
                    abbreviation = xml.readElementText();
                }
                else {
                    // skip over this tag
                    xml.skipCurrentElement();
                }
            }

            if (id> 0 && !name.isEmpty()>0 && !abbreviation.isEmpty()){
                qDebug() << id << abbreviation;
            }
        }
    }

    return true;
}

void TheTVDBScraper::internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language)  {
    Q_UNUSED(manager);
    Q_UNUSED(mediaMovieSearchPtr);
    Q_UNUSED(searchFor);
    Q_UNUSED(language);

    emit scraperError("Unsupported Operation");
}

bool TheTVDBScraper::parseEpisode(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode, bool& foundEpisode){

    //
    // Cf. http://thetvdb.com/wiki/index.php?title=API:Base_Episode_Record
    //

    if(xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "Episode") {
        return false;
    }

    int episodeNumber=-1;
    int seasonNumber=-1;
    QString overview;
    QString filename;
    QString thumb_height;
    QString thumb_width;
    QString rating;
    QString episodeTitle;
    QStringList directors;
    QDateTime firstAired;

    while(!(xml.tokenType() == QXmlStreamReader::EndElement &&  xml.name() == "Episode")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if ( xml.name() == QLatin1String( "EpisodeNumber" ) ) {
                episodeNumber=xml.readElementText().toInt();
            } else if ( xml.name() == QLatin1String( "SeasonNumber" ) ) {
                seasonNumber=xml.readElementText().toInt();
            }else if ( xml.name() == QLatin1String( "Overview" ) ) {
                overview=xml.readElementText();
            }else if ( xml.name() == QLatin1String( "filename" ) ) {
                filename=xml.readElementText();
            }else if ( xml.name() == QLatin1String( "thumb_height" ) ) {
                thumb_height=xml.readElementText();
            }else if ( xml.name() == QLatin1String( "thumb_width" ) ) {
                thumb_width=xml.readElementText();
            } else if ( xml.name() == QLatin1String( "Rating" ) ) {
                rating=xml.readElementText();
            } else if ( xml.name() == QLatin1String( "EpisodeName" ) ) {
                episodeTitle=xml.readElementText();
            } else if ( xml.name() == QLatin1String( "Director" ) ) {
                directors =xml.readElementText().split('|',QString::SkipEmptyParts);
            } else if ( xml.name() == QLatin1String( "FirstAired" ) ) {
                QDate date = QDate::fromString(xml.readElementText(), "yyyy-MM-dd");
                if (date.isValid() ){
                    firstAired = QDateTime(date);
                }
            }
        }

        xml.readNext();

    }

    if (episodeNumber==episode&&season==seasonNumber){
        mediaTVSearchPtr->setSynopsis(overview);
        mediaTVSearchPtr->setEpisodeRating(rating.isEmpty()?-1:rating.toDouble());
        mediaTVSearchPtr->setEpisodeTitle(episodeTitle);
        mediaTVSearchPtr->setSeason(season);
        mediaTVSearchPtr->setEpisode(episode);
        mediaTVSearchPtr->setDirectors(directors);
        mediaTVSearchPtr->setAired(firstAired);
        mediaTVSearchPtr->addThumbail(filename,QSize(thumb_width.toInt(),thumb_height.toInt()));

        foundEpisode= true;
    } else {
        foundEpisode=false;
    }

    return true;
}

bool TheTVDBScraper::parseSeries(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode ){
    Q_UNUSED(season);
    Q_UNUSED(episode);

    if(xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "Series") {
        return false;
    }

    QString runtime;
    QStringList genre;

    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Series")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if ( xml.name() == QLatin1String( "SeriesName" ) ) {
                mediaTVSearchPtr->setTitle(xml.readElementText());
            } else if ( xml.name() == QLatin1String( "Runtime" ) ) {
                runtime=xml.readElementText();
            } else if ( xml.name() == QLatin1String( "Network" ) ) {
                mediaTVSearchPtr->addNetwork(xml.readElementText());
            } else if ( xml.name() == QLatin1String( "Genre" ) ) {
                genre =xml.readElementText().split('|',QString::SkipEmptyParts);
            } else if ( xml.name() == QLatin1String( "Rating" ) ) {
                mediaTVSearchPtr->setShowRating(xml.readElementText().toDouble());
            }
        }
        xml.readNext();
    }


    mediaTVSearchPtr->setGenre(genre);

    int runtimeAsInt=runtime.toInt()*60;
    if (runtimeAsInt>0){
        mediaTVSearchPtr->setRuntimeInSec(runtimeAsInt);
    }

    return true;
}

bool TheTVDBScraper::parseInfo(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode){
    qDebug() << data;
    QXmlStreamReader xml( data );
    bool episodeFound=false;

    while(!xml.atEnd() &&
          !xml.hasError()) {

        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if(token == QXmlStreamReader::StartElement) {
            if(xml.name() == "Data") {
                continue;
            } else  if(xml.name() == "Series") {
                if (!parseSeries(xml,mediaTVSearchPtr, season, episode)){
                    return false;
                }
            } else if(xml.name() == "Episode" && !episodeFound) {
                if (!parseEpisode(xml,mediaTVSearchPtr, season, episode,episodeFound)){
                    return false;
                }
            }
        }
    }

    return true;
}

bool TheTVDBScraper::parseBanner(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr){
    QXmlStreamReader xml( data );

    if ( xml.readNextStartElement() && xml.name()==QLatin1String("Banners")){
        while ( xml.readNextStartElement()) {
            if (xml.name() == QLatin1String( "Banner" ) ) {
                QString bannerPath;
                QString bannerType;
                QString bannerType2;
                while ( xml.readNextStartElement() ) {
                    if ( xml.name() == QLatin1String( "id" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "BannerPath" ) ) {
                        bannerPath=  xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "BannerType" ) ) {
                        bannerType= xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "BannerType2" ) ) {
                        bannerType2=xml.readElementText();
                    } else if ( xml.name() == QLatin1String( "Colors" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "Language" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "RatingCount" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "SeriesName" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "ThumbnailPath" ) ) {
                        xml.skipCurrentElement();
                    } else if ( xml.name() == QLatin1String( "VignettePath" ) ) {
                        xml.skipCurrentElement();
                    } else {
                        xml.skipCurrentElement();
                    }
                }

                if (!bannerPath.isEmpty()){
                    if (bannerType==QLatin1String( "series" ) && bannerType2==QLatin1String( "graphical")){
                        // Cf. http://www.thetvdb.com/wiki/index.php?title=Series_Banners
                        mediaTVSearchPtr->addBanner(bannerPath,QSize(758,140));
                    } else if (bannerType==QLatin1String( "fanart" ))  {

                        QRegularExpression re("(\\d+)x(\\w+)");
                        QRegularExpressionMatch match = re.match(bannerType2);
                        QSize backdropSize;
                        if (match.hasMatch()) {
                            int w = match.captured(1).toInt();
                            int h = match.captured(2).toInt();
                            backdropSize=QSize(w,h);
                        } else {
                            // todo
                            backdropSize=QSize();
                        }

                        mediaTVSearchPtr->addBackdrop(bannerPath,backdropSize);


                    } else if (bannerType==QLatin1String( "poster" ))  {

                        QRegularExpression re("(\\d+)x(\\w+)");
                        QRegularExpressionMatch match = re.match(bannerType2);
                        QSize posterSize;
                        if (match.hasMatch()) {
                            int w = match.captured(1).toInt();
                            int h = match.captured(2).toInt();
                            posterSize=QSize(w,h);
                        } else {
                            // Cf. http://www.thetvdb.com/wiki/index.php?title=Posters
                            posterSize=QSize( 680,1000 );
                        }

                        mediaTVSearchPtr->addPoster(bannerPath,posterSize);

                    }

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

bool TheTVDBScraper::parseActors(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr){
    //
    // Cf. http://thetvdb.com/wiki/index.php?title=API:actors.xml
    //
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

        mediaTVSearchPtr->setActors(mediaTVSearchPtr->actors() << actors[0] << actors[1] << actors[2] << actors[3]);

        return true;
    } else {
        return false;
    }
}

void TheTVDBScraper::internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language) {

    Q_UNUSED(searchFor);

    QString showCode= mediaTVSearchPtr->foundResult().getCode();
    int season=mediaTVSearchPtr->foundResult().getSeason();
    int episode=mediaTVSearchPtr->foundResult().getEpisode();

    QString lang=language;
    QString command=QString::fromLatin1( "%1/series/%2/all/%3.zip" ).arg(API_KEY).arg( showCode ).arg( lang ) ;

    QMap<QString,QString> params;
    QString url=createURL(command,params);
    Promise* promise=Promise::loadAsync(*manager,url,false);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            QByteArray data= promise->replyData();
            QBuffer buffer(&data);
            QuaZip replyZip(&buffer);
            replyZip.open(QuaZip::mdUnzip);

            for(bool f=replyZip.goToFirstFile(); f; f=replyZip.goToNextFile()) {
                QuaZipFile zFile( &replyZip);

                zFile.open( QIODevice::ReadOnly );

                QString currentFileName=replyZip.getCurrentFileName();
                if (currentFileName.endsWith("banners.xml",Qt::CaseInsensitive)){
                    QByteArray bannersArray=zFile.readAll();
                    qDebug() << bannersArray;
                    parseBanner( bannersArray, mediaTVSearchPtr);
                }else if (currentFileName.endsWith("actors.xml",Qt::CaseInsensitive)){
                    QByteArray actorsArray=zFile.readAll();
                    qDebug() << actorsArray;
                    parseActors( actorsArray, mediaTVSearchPtr);
                } else if (currentFileName.endsWith(QString("%1.xml").arg(lang),Qt::CaseInsensitive)){
                    QByteArray infoArray=zFile.readAll();
                    qDebug() << infoArray;

                    parseInfo( infoArray, mediaTVSearchPtr,season, episode);
                }

                zFile.close();

            }

            replyZip.close();

            emit found(this,mediaTVSearchPtr);
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}


void TheTVDBScraper::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year)  {
    Q_UNUSED(manager);
    Q_UNUSED(toSearch);
    Q_UNUSED(language);
    Q_UNUSED(year);

    emit scraperError("Unsupported Operation");
}

void TheTVDBScraper::retriveLanguages(QNetworkAccessManager* manager)  {

    QString url = QString("http://thetvdb.com/api/%1/languages.xml").arg(API_KEY);
    Promise* promise=Promise::loadAsync(*manager,url,false);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            QByteArray data= promise->replyData();
            if (parseLanguageList(data)){
                // Ok here
            }
        }
        // Ko here
    });

}

void TheTVDBScraper::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) {
    if  (retrieveMirror){
        QString url = QString("http://thetvdb.com/api/%1/mirrors.xml").arg(API_KEY);
        Promise* promise=Promise::loadAsync(*manager,url,false);
        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray data= promise->replyData();
                if (parseMirrorList(data)){
                    retrieveMirror= false;
                    internalSearchTV(manager,toSearch,language);
                } else {
                    emit scraperError(tr("Unable to parse 'mirrors.xml'"));
                }
            } else {
                emit scraperError(tr("Unable to retrieve 'mirrors.xml'"));
            }
        });
    }  else {
        QMap<QString,QString> params;
        params["language"]=QUrl::toPercentEncoding(language);
        params["seriesname"]=QUrl::toPercentEncoding(toSearch);
        QString url=createURL("GetSeries.php",params);
        Promise* promise=Promise::loadAsync(*manager,url,false);
        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray data= promise->replyData();
                qDebug() << data;
                parseSeriesList(manager,toSearch, data,language);
            }else {
                emit scraperError(promise->replyErrorString());
            }
        });
    }
}

QString TheTVDBScraper::getBestImageUrl(const QString& url, const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode, QFlags<ImageType> imageType) const {

    QSize trueSize=size;
    if (originalSize.isValid()){
        trueSize=originalSize.scaled(size,mode);
    }

    // Can we use _cache?
    static QSize cachedBanner(300,55);
    static QSize cachedFanart(300,168);
    static QSize cachedPoster(300,411);

    QSize cachedSize;

    if (imageType & ImageType::Banner){
        cachedSize = cachedBanner;
    } else if (imageType & ImageType::Poster){
        cachedSize = cachedPoster;
    } else if (imageType & ImageType::Backdrop){
        cachedSize = cachedFanart;
    }

    bool useCache=false;
    if (cachedSize.isValid()){
        // size is lower than cachedSize -> we can use _cache
        useCache=(trueSize.boundedTo(cachedSize)==trueSize);
    }

    if (useCache){
        return getBannerURL().append("/banners/_cache/").append(url);
    } else {
        return getBannerURL().append("/banners/").append(url);
    }

}

QIcon TheTVDBScraper::loadIcon() const {

    static const uchar icon_png[] = {
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

    QPixmap pixmap;
    if (pixmap.loadFromData(icon_png, sizeof(icon_png)/sizeof(uchar))){
        return QIcon(pixmap);
    }

    return QIcon();
}

QString TheTVDBScraper::name() const {
    return QStringLiteral("TVDB");
}

QIcon TheTVDBScraper::icon() const {
    return m_icon;
}

bool TheTVDBScraper::haveCapability(const SearchCapabilities capability) const {
    return capability==TV;
}

bool TheTVDBScraper::supportLanguage(const QString& languageCodeISO639) const {
    Q_UNUSED(languageCodeISO639);
    return true;
}
