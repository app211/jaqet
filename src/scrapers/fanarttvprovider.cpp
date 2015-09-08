#include "fanarttvprovider.h"

#include "themoviedbscraper.h"
#include "thetvdbscraper.h"
#include "promise.h"

#include <QUrl>

namespace  {
const QString API_KEY(QStringLiteral("7bda601d79accbbef43d11a030908864"));
const QString URL(QStringLiteral("url"));
}

FanArtTVProvider::FanArtTVProvider(QObject *parent)
    :Scraper(parent), m_icon(loadIcon())
{ 
    m_tvdbscraper = new TheTVDBScraper(this);

    connect(m_tvdbscraper, SIGNAL(found(ShowPtrList)), this,
            SLOT(_found(ShowPtrList)));


    connect(m_tvdbscraper, SIGNAL(scraperError()), this,
            SLOT(_scraperError()));

    connect(m_tvdbscraper, SIGNAL(scraperError(const QString&)), this,
            SLOT(_scraperError(const QString&)));

    m_moviedbscraper= new TheMovieDBScraper(this);

    connect(m_moviedbscraper, SIGNAL(found(FilmPrtList)), this,
            SLOT(_found(FilmPrtList)));


    connect(m_tvdbscraper, SIGNAL(scraperError()), this,
            SLOT(_scraperError()));

    connect(m_tvdbscraper, SIGNAL(scraperError(const QString&)), this,
            SLOT(_scraperError(const QString&)));

}

void FanArtTVProvider::_scraperError(const QString& error){
    emit scraperError(error);
}

void FanArtTVProvider::_scraperError(){
    emit scraperError();
}

void FanArtTVProvider::_found(FilmPrtList films){
    Q_ASSERT(sender()==m_moviedbscraper);

    for (FilmPtr film : films){
        film->posterHref="";
    }

    emit found(films);
}

void FanArtTVProvider::_found(ShowPtrList shows){
    Q_ASSERT(sender()==m_tvdbscraper);

    for (ShowPtr show : shows){
        show->posterHref="";
    }

    emit found(shows);
}

QString FanArtTVProvider::createURL(const QString& command, const QMap<QString, QString>& params) const {

    QString url=QString().append("http://webservice.fanart.tv/").append(command).append("?api_key=").append(QUrl::toPercentEncoding(API_KEY));

    foreach( QString key, params.keys() )
    {
        url.append(QString("&%1=%2").arg(QUrl::toPercentEncoding(key),params.value( QUrl::toPercentEncoding(key))));
    }

    return url;
}

bool FanArtTVProvider::isImageForLanguage(const QJsonObject& imageObject, const QString& language){
    QString objectLangString=imageObject["lang"].toString();
    return objectLangString.isEmpty() || QString::compare(QStringLiteral("en"),objectLangString,Qt::CaseInsensitive)==0  || QString::compare(language,objectLangString,Qt::CaseInsensitive)==0;
}

bool FanArtTVProvider::isImageForLanguageAndSeason(const QJsonObject& imageObject, const QString& language, const int season){

    QString objectSeasonString=imageObject["season"].toString();
    bool useObject=false;
    if (!objectSeasonString.isEmpty() && objectSeasonString!="all"){
        bool bOk;
        int objectSeason=objectSeasonString.toInt(&bOk);
        if (bOk && objectSeason==season){
            useObject = true;
        }
    } else {
        useObject = true;
    }

    if (!useObject){
        return false;
    }

    return isImageForLanguage(imageObject,language);
}

bool FanArtTVProvider::parseResult(const QJsonDocument& resultset,  MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language, const int season) {
    if (!resultset.isObject()){
        return false;
    }

    QJsonObject jsonObject = resultset.object();

    if (searchFor & SearchOption::Information){
        mediaTVSearchPtr->setOriginalTitle(jsonObject[QStringLiteral("name")].toString());
    }

    if (searchFor & SearchOption::BackDrop){
        if (jsonObject[QStringLiteral("showbackground")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("showbackground")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguageAndSeason(imageObject,language, season);

                if (useImage){
                    mediaTVSearchPtr->addBackdrop(imageObject[URL].toString());
                }

            }
        }
    }

    if (searchFor & SearchOption::Poster){

        if (jsonObject[QStringLiteral("tvposter")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("tvposter")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguageAndSeason(imageObject,language,season);

                if (useImage){
                    mediaTVSearchPtr->addPoster(imageObject[URL].toString());

                }

            }
        }


        if (jsonObject[QStringLiteral("seasonposter")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("seasonposter")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguageAndSeason(imageObject,language,season);

                if (useImage){
                    mediaTVSearchPtr->addPoster(imageObject[URL].toString());

                }

            }
        }
    }

    if (searchFor & SearchOption::Banner){

        if (jsonObject[QStringLiteral("tvbanner")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("tvbanner")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguageAndSeason(imageObject,language,season);

                if (useImage){
                    mediaTVSearchPtr->addBanner(imageObject[URL].toString());
                }
            }
        }

        if (jsonObject[QStringLiteral("seasonbanner")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("seasonbanner")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguageAndSeason(imageObject,language,season);

                if (useImage){
                    mediaTVSearchPtr->addBanner(imageObject[URL].toString());
                }
            }
        }
    }
    return true;
}



void FanArtTVProvider::internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language)  {

    QString showCode= mediaTVSearchPtr->foundResult().getCode();
    int season=mediaTVSearchPtr->foundResult().getSeason();
    int episode=mediaTVSearchPtr->foundResult().getEpisode();

    QMap<QString,QString> params;
    if (!m_personalApiKey.isEmpty()){
        params["client_key"]=m_personalApiKey;
    }

    QString url=createURL(QStringLiteral("v3/tv/%1").arg(showCode),params);
    Promise* promise=Promise::loadAsync(*manager, url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            QJsonParseError e;
            QByteArray data=promise->replyData();
            qDebug() << data;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                if (searchFor & SearchOption::Information){
                    mediaTVSearchPtr->setEpisode(episode);
                    mediaTVSearchPtr->setSeason(season);
                }
                if (parseResult(doc, mediaTVSearchPtr,searchFor,language, season)){
                    emit found(this,mediaTVSearchPtr);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }

        }  else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

bool FanArtTVProvider::parseResult(const QJsonDocument& resultset,  MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) {
    if (!resultset.isObject()){
        return false;
    }

    QJsonObject jsonObject = resultset.object();

    if (searchFor & SearchOption::Information){
        mediaMovieSearchPtr->setOriginalTitle(jsonObject[QStringLiteral("name")].toString());
    }

    if (searchFor & SearchOption::BackDrop){
        if (jsonObject[QStringLiteral("moviebackground")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("moviebackground")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguage(imageObject,language);

                if (useImage){
                    mediaMovieSearchPtr->addBackdrop(imageObject[URL].toString());
                }

            }
        }
    }

    if (searchFor & SearchOption::Thumbnail){
        if (jsonObject[QStringLiteral("moviethumb")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("moviethumb")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguage(imageObject,language);

                if (useImage){
                    mediaMovieSearchPtr->addThumbail(imageObject[URL].toString());
                }

            }
        }
    }

    if (searchFor & SearchOption::Poster){
        if (jsonObject[QStringLiteral("movieposter")].isArray()){

            for (const QJsonValue & imageValue : jsonObject[QStringLiteral("movieposter")].toArray())
            {
                QJsonObject imageObject = imageValue.toObject();

                bool useImage=isImageForLanguage(imageObject,language);

                if (useImage){
                    mediaMovieSearchPtr->addPoster(imageObject[URL].toString());
                }
            }
        }
    }

    return true;
}
void FanArtTVProvider::internalFindMovieInfo(QNetworkAccessManager* manager,  MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) {
    QString showCode= mediaMovieSearchPtr->foundResult().getCode();
    int productionYear=mediaMovieSearchPtr->foundResult().getProductionYear().toInt();

    QMap<QString,QString> params;
    if (!m_personalApiKey.isEmpty()){
        params["client_key"]=m_personalApiKey;
    }

    QString url=createURL(QStringLiteral("v3/movies/%1").arg(showCode),params);
    Promise* promise=Promise::loadAsync(*manager, url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            QJsonParseError e;
            QByteArray data=promise->replyData();
            qDebug() << data;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                if (searchFor & SearchOption::Information && productionYear>0){
                    mediaMovieSearchPtr->setProductionYear(productionYear);
                }

                if (parseResult(doc, mediaMovieSearchPtr,searchFor,language)){
                    emit found(this,mediaMovieSearchPtr);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }

        }  else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

void FanArtTVProvider::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const{
    m_tvdbscraper->searchTV(manager,toSearch,language);
}

void FanArtTVProvider::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const {
    m_moviedbscraper->searchFilm(manager,toSearch,year,language);
}

QString FanArtTVProvider::getBestImageUrl(const QString& url, const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode, QFlags<ImageType> imageType) const {
    Q_UNUSED(originalSize);
    Q_UNUSED(size);
    Q_UNUSED(mode);
    Q_UNUSED(imageType);

    qDebug() << url;
    qDebug() << url;
    qDebug() << url;
    qDebug() << url;
    qDebug() << url;
    qDebug() << url;

    return url;
}

QIcon FanArtTVProvider::loadIcon() const {

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

QString FanArtTVProvider::name() const {
    return QStringLiteral("Fanart.tv");
}

QIcon FanArtTVProvider::icon() const {
    return m_icon;
}

bool FanArtTVProvider::supportLanguage(const QString& languageCodeISO639) const {
    Q_UNUSED(languageCodeISO639);
    return true;
}

bool FanArtTVProvider::haveCapability(const SearchCapabilities capability) const{
    Q_UNUSED(capability);
    return true;
}
