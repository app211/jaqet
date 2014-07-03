
#include "allocinescraper.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QNetworkReply>
#include <QApplication>

#include "../promise.h"


AlloCineScraper::AlloCineScraper(QObject *parent)
    :Scraper(parent)
{
}

QString AlloCineScraper::createURL(const QString& type, const QMap<QString, QString> &params) const
{

    QString mapParams;
    foreach( QString key, params.keys() )
    {
         mapParams.append(QString("&%1=%2").arg(QUrl::toPercentEncoding(key),params.value( QUrl::toPercentEncoding(key))));
    }

    QString searchQuery = QString("partner=100043982026&format=json%2&sed=%3").arg(mapParams,QDate::currentDate().toString("yyyyMMdd"));

    QString toEncrypt = "29d185d98c984a359e6e6f26a0474269"+searchQuery;

    QString fullQuery= QString("http://api.allocine.fr/%2?%3&sig=%4").arg( type, searchQuery, QUrl::toPercentEncoding(QCryptographicHash::hash(toEncrypt.toUtf8(), QCryptographicHash::Sha1).toBase64()));

    qDebug() << fullQuery;

    return fullQuery;
}

void AlloCineScraper::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString &language) const
{
    QMap<QString,QString> params;
    params["filter"]=QUrl::toPercentEncoding("movie");
    params["q"]=QUrl::toPercentEncoding(toSearch);

    QString url=createURL("rest/v3/search",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                emit found(parseResultset(doc));
            } else {
                emit scraperError();
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}


void AlloCineScraper::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const {

    QMap<QString,QString> params;
    params["filter"]="tvseries";
    params["q"]=QUrl::toPercentEncoding(toSearch);

    QString url=createURL("rest/v3/search",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                emit found(parseTVResultset(doc));
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}

void  AlloCineScraper::internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const{
    QMap<QString,QString> params;
    params["filter"]=QUrl::toPercentEncoding("movie");
    params["code"]=QUrl::toPercentEncoding(movieCode);
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QString url=createURL("rest/v3/movie",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                SearchMovieInfo rseult;
                if(parseMovieInfo(doc,rseult)){
                    emit found(this, rseult);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}


void AlloCineScraper::internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const {

    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(showCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QString url=createURL("rest/v3/tvseries",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
            qDebug() << data;
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                QString seasonCode;
                SearchEpisodeInfo result;
                result.episode=episode;
                result.season=season;
                if(extractSeasonCodeFromLargeTVSerieInfo(doc,season,seasonCode,result)){
                    findSeasonInfoByCode(manager, seasonCode, episode,result);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}

bool AlloCineScraper::extractEpisodeCodeFromLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode, SearchEpisodeInfo& result) const{


    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["season"].isObject()){
        return false;
    }

    QJsonObject season = jsonObject["season"].toObject();


    if (!season["episode"].isArray()){
        return false;
    }

    QJsonArray episodes = season["episode"].toArray();

    foreach (const QJsonValue & value, episodes)
    {
        QJsonObject episode = value.toObject();
        if(episode["episodeNumberSeason"].toInt()==episodeToFind){
            episodeCode.setNum(episode["code"].toInt());
            result.productionYear = season["yearStart"].toInt();
             return true;
        }
    }

    return false;
}

bool AlloCineScraper::extractSeasonCodeFromLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString& seasonCode, SearchEpisodeInfo& result) const{


    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["tvseries"].isObject()){
        return false;
    }

    QJsonObject tvseriesObject = jsonObject["tvseries"].toObject();


    if (!tvseriesObject["season"].isArray()){
        return false;
    }

    QJsonArray jsonArray = tvseriesObject["season"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject season = value.toObject();
        if(season["seasonNumber"].toInt()==seasonToFind){
            seasonCode.setNum(season["code"].toInt());
            result.title=tvseriesObject["title"].toString();
            result.originalTitle=tvseriesObject["originalTitle"].toString();
            return true;
        }
    }


    return false;
}


void AlloCineScraper::findSeasonInfoByCode(QNetworkAccessManager *manager,const QString seasonCode,const int episode, SearchEpisodeInfo result) const{

    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(seasonCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QString url=createURL("rest/v3/season",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
//                qDebug() << data;
                QString episodeCode;
                SearchEpisodeInfo result2=result;
                if(extractEpisodeCodeFromLargeSeasonTVSerieInfo(doc,episode,episodeCode, result2)){
                   findEpisodeInfoByCode(manager,episodeCode, result2);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}

bool parseEpisodeTVSerieInfo(const QJsonDocument& resultset, SearchEpisodeInfo& result ){

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["episode"].isObject()){
        return false;
    }

    QJsonObject episodeObject = jsonObject["episode"].toObject();

    result.synopsis=episodeObject["synopsis"].toString();
    result.code=episodeObject["code"].toString();
    result.episodeTitle=episodeObject["title"].toString();
    result.originalEpisodeTitle=episodeObject["originalTitle"].toString();

    QJsonArray jsonArray = episodeObject["link"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject link = value.toObject();

        if (link["rel"].toString()=="aco:web"){
            result.linkName=link["name"].toString();
            if (result.linkName.isEmpty()){
                result.linkName=result.title;
            }
            result.linkHref=link["href"].toString();
            break;
        }
    }

    return true;
}


void AlloCineScraper::findEpisodeInfoByCode(QNetworkAccessManager *manager, const QString episodeCode, SearchEpisodeInfo result) const{

    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(episodeCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QString url=createURL("rest/v3/episode",params);
    Promise* promise=Promise::loadAsync(*manager,url);
    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->reply->error() ==QNetworkReply::NoError){
            const QByteArray data=promise->reply->readAll();
        //    qDebug() << data;
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
               SearchEpisodeInfo result2=result;
                if (parseEpisodeTVSerieInfo(doc, result2)){
                     emit found(this,result2);
                }else {
                     emit scraperError();
                }
             } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->reply->errorString());
        }
    });
}


bool AlloCineScraper::parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const{

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["movie"].isObject()){
        return false;
    }

    QJsonObject movieObject = jsonObject["movie"].toObject();

    info.synopsis=movieObject["synopsis"].toString();
    if(movieObject["poster"].isObject()){
        info.posterHref = movieObject["poster"].toObject()["href"].toString();
        info.postersHref.append(info.posterHref);
    }

    if(movieObject["castingShort"].isObject()){
        info.directors=movieObject["castingShort"].toObject()["directors"].toString().split(",", QString::SkipEmptyParts);
        info.directors.replaceInStrings(QRegExp("^\\s+"),"");
        info.actors=movieObject["castingShort"].toObject()["actors"].toString().split(",", QString::SkipEmptyParts);
        info.actors.replaceInStrings(QRegExp("^\\s+"),"");
    }

    info.productionYear = movieObject["productionYear"].toInt();
    info.runtime = movieObject["runtime"].toInt();

    QJsonArray jsonArray = movieObject["link"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject link = value.toObject();

        if (link["rel"].toString()=="aco:web"){
            info.linkName=link["name"].toString();

            info.linkHref=link["href"].toString();
            break;
        }
    }

    return true;
}

QString AlloCineScraper::getBestImageUrl(const QString& filePath, const QSize& , ImageType imageType) const{
    return filePath;
}

FilmPrtList AlloCineScraper::parseResultset(const QJsonDocument& resultset) const{

    FilmPrtList films;

    if (!resultset.isObject()){
        return films;
    }

    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["feed"].isObject()){
        return films;
    }

    QJsonObject feedObject = jsonObject["feed"].toObject();

    if (!feedObject["movie"].isArray()){
        return films;
    }

    QJsonArray jsonArray = feedObject["movie"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        FilmPtr film(new Film());

        film->originalTitle= obj["originalTitle"].toString();
        film->title= obj["title"].toString();
        if (film->title.isEmpty()){
            film->title=film->originalTitle;
        }
        film->productionYear = QString::number(obj["productionYear"].toDouble());
        film->code= QString::number(obj["code"].toDouble());
        if(obj["poster"].isObject()){
            film->posterHref = obj["poster"].toObject()["href"].toString();
        }



        films.append(film);
    }

    return films;
}

ShowPtrList AlloCineScraper::parseTVResultset(const QJsonDocument& resultset) const{

    ShowPtrList shows;

    if (!resultset.isObject()){
        return shows;
    }

    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["feed"].isObject()){
        return shows;
    }

    QJsonObject feedObject = jsonObject["feed"].toObject();

    if (!feedObject["tvseries"].isArray()){
        return shows;
    }

    QJsonArray jsonArray = feedObject["tvseries"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        ShowPtr show(new Show());
        show->originalTitle= obj["originalTitle"].toString();
        show->title= obj["originalTitle"].toString();
        show->productionYear = QString::number(obj["yearStart"].toDouble());
        show->code= QString::number(obj["code"].toDouble());
        if(obj["poster"].isObject()){
            show->posterHref = obj["poster"].toObject()["href"].toString();
        }

        shows.append(show);
    }

    return shows;
}

const uchar AlloCineScraper::icon_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
    0x88, 0x00, 0x00, 0x07, 0x86, 0x49, 0x44, 0x41, 0x54, 0x58, 0x85, 0x75, 0x97, 0x7b, 0x8c, 0x54,
    0x57, 0x1d, 0xc7, 0x3f, 0xe7, 0xdc, 0x3b, 0x33, 0xbb, 0x33, 0x3b, 0x65, 0x17, 0x84, 0xdd, 0xb0,
    0xbc, 0xd9, 0xb2, 0x20, 0x2c, 0xa5, 0x69, 0x61, 0xd5, 0x34, 0x05, 0x01, 0x45, 0xad, 0x89, 0x60,
    0xaa, 0xd1, 0xf6, 0x0f, 0xad, 0x26, 0x35, 0x31, 0x8a, 0x86, 0xc4, 0xd7, 0x1f, 0xc6, 0xc4, 0xc4,
    0xfe, 0xa3, 0xb6, 0x6a, 0x9a, 0xda, 0xc4, 0xb7, 0x96, 0x26, 0xd2, 0x94, 0xd8, 0xa4, 0x42, 0xb4,
    0x56, 0xac, 0x74, 0xe9, 0x73, 0x69, 0x59, 0x0a, 0x48, 0x83, 0x74, 0xd9, 0x6e, 0x61, 0x5f, 0xb0,
    0x8f, 0x79, 0xde, 0xb9, 0xf7, 0x9c, 0x9f, 0x7f, 0xdc, 0x3b, 0x33, 0x77, 0x86, 0xf1, 0x4c, 0x6e,
    0xee, 0x39, 0x77, 0x4e, 0xce, 0xf7, 0x7b, 0x7e, 0x8f, 0xef, 0xf9, 0x1d, 0x25, 0x17, 0x06, 0x85,
    0xff, 0xdb, 0x54, 0x53, 0x3f, 0x1a, 0xab, 0xf8, 0x5b, 0xb5, 0x9e, 0x8e, 0xb4, 0xf8, 0x56, 0xfd,
    0x5c, 0x87, 0x74, 0xe3, 0x83, 0x9b, 0xc1, 0x25, 0xd6, 0xaf, 0x76, 0xab, 0xa0, 0x0a, 0xd0, 0x4d,
    0x84, 0xaa, 0xf3, 0xa5, 0xfe, 0xad, 0x61, 0x7d, 0xa9, 0x8f, 0x25, 0xec, 0xbb, 0x88, 0x6d, 0x01,
    0x5c, 0x5d, 0x44, 0x37, 0x82, 0xa9, 0x2a, 0xa8, 0x0a, 0xff, 0x53, 0x2a, 0x06, 0xde, 0x0c, 0x22,
    0xad, 0xc7, 0x2a, 0x02, 0x8f, 0xf6, 0xe7, 0x42, 0x9c, 0x40, 0x75, 0xd7, 0x71, 0xd3, 0x46, 0x60,
    0x68, 0x8c, 0x38, 0x5c, 0x1c, 0x85, 0x91, 0x4b, 0x8a, 0x89, 0xd9, 0x76, 0x4a, 0x7e, 0x06, 0x54,
    0x82, 0x64, 0xd2, 0x92, 0x49, 0x96, 0xe8, 0x59, 0x52, 0x62, 0xdb, 0x06, 0x58, 0xbd, 0x5c, 0x50,
    0x35, 0x02, 0x36, 0x5a, 0xd3, 0x52, 0xdf, 0x6c, 0x44, 0x48, 0x49, 0x9c, 0x40, 0x0c, 0x5c, 0x35,
    0x82, 0xe7, 0x4b, 0x0e, 0x47, 0xff, 0xa9, 0x38, 0x71, 0x3a, 0x4d, 0xc9, 0xcf, 0x92, 0x6e, 0xcf,
    0xa0, 0xb5, 0xc6, 0x5a, 0x41, 0x39, 0x9a, 0x64, 0x32, 0x49, 0x36, 0x9b, 0x65, 0xa2, 0x98, 0x61,
    0x78, 0x34, 0x20, 0x93, 0x9a, 0xe6, 0xf6, 0x75, 0x97, 0xd9, 0x3b, 0x08, 0x8e, 0xb6, 0xa0, 0x2c,
    0x88, 0x0a, 0x77, 0x2e, 0x31, 0x17, 0x88, 0x45, 0xc9, 0xb9, 0xad, 0xd2, 0x6a, 0xb7, 0x28, 0x07,
    0x70, 0x39, 0x31, 0xac, 0x79, 0xf4, 0x88, 0x4b, 0x25, 0xe8, 0x40, 0x6b, 0x1d, 0x99, 0x4f, 0xa3,
    0x75, 0xf8, 0x28, 0xad, 0xd1, 0x8e, 0xc6, 0x71, 0x5c, 0xb2, 0x19, 0x45, 0xc5, 0xa4, 0x58, 0xbc,
    0x64, 0x09, 0xcb, 0x7b, 0x97, 0x63, 0xca, 0x17, 0xb8, 0xff, 0xa3, 0xd7, 0x58, 0xb1, 0x0c, 0x90,
    0x00, 0xc4, 0x03, 0x5b, 0x01, 0x1b, 0xd4, 0x2c, 0xa2, 0xe4, 0xad, 0x2d, 0x52, 0xdf, 0xb5, 0x02,
    0x1c, 0x50, 0x0e, 0x82, 0xcb, 0xe3, 0x47, 0x35, 0x4f, 0x3f, 0xdf, 0x86, 0xeb, 0x26, 0x42, 0x7a,
    0x91, 0x65, 0x94, 0x52, 0x68, 0xa5, 0x41, 0x6b, 0xb6, 0x6e, 0xa8, 0xb0, 0x7d, 0xa0, 0xcc, 0xc0,
    0x06, 0x8f, 0xb5, 0xbd, 0x15, 0x0e, 0xfd, 0xf4, 0x56, 0x6e, 0x2c, 0x64, 0x48, 0xb5, 0x25, 0x59,
    0xdf, 0xd7, 0x47, 0x3a, 0x2d, 0xdc, 0x33, 0x78, 0x89, 0xbe, 0xde, 0x0a, 0x98, 0x1b, 0x60, 0x17,
    0xc0, 0x7a, 0x60, 0x0d, 0x88, 0x41, 0x87, 0x2e, 0xb0, 0x75, 0xd3, 0x28, 0x40, 0x69, 0x1e, 0x3d,
    0x02, 0x47, 0x9e, 0x4b, 0x02, 0x0a, 0x13, 0x18, 0x4c, 0x60, 0x10, 0x1b, 0x60, 0x82, 0x80, 0x20,
    0x08, 0xf0, 0xfd, 0x00, 0xbf, 0xe2, 0xf3, 0xc0, 0xa7, 0xa6, 0x39, 0xf0, 0xe1, 0x59, 0xfa, 0x56,
    0x94, 0x70, 0xb4, 0xa5, 0x6f, 0xe5, 0x3c, 0x65, 0xaf, 0x8c, 0x57, 0xf1, 0xb9, 0xfa, 0xde, 0x55,
    0x92, 0xc9, 0x4e, 0x46, 0x17, 0xf6, 0x33, 0x1f, 0x6c, 0x06, 0x77, 0x31, 0xa8, 0x64, 0x14, 0xbc,
    0x61, 0x1c, 0xe8, 0x78, 0x40, 0x54, 0x19, 0x1c, 0x3f, 0x05, 0x4f, 0x3d, 0x17, 0x66, 0xe8, 0xfa,
    0x15, 0x65, 0xee, 0xfd, 0xc8, 0x3c, 0x3f, 0x3a, 0x38, 0xc5, 0xd3, 0x0f, 0x8f, 0xb3, 0x71, 0x6d,
    0x31, 0x04, 0x0f, 0x7c, 0x96, 0x2f, 0x2b, 0xb1, 0xb6, 0xd7, 0xc3, 0x58, 0x4b, 0xb1, 0x24, 0x88,
    0x08, 0x9b, 0xd6, 0xe4, 0xf1, 0x3c, 0x0f, 0xcf, 0xf3, 0x08, 0x82, 0x80, 0x42, 0xbe, 0x48, 0x57,
    0x57, 0x0f, 0x67, 0xc7, 0xfa, 0x09, 0x6c, 0x7b, 0xe8, 0xda, 0x9a, 0x16, 0x54, 0xd3, 0x50, 0xa9,
    0x30, 0x48, 0x14, 0xcc, 0xe6, 0x14, 0x8f, 0x3c, 0x11, 0xba, 0xc3, 0x18, 0xc3, 0xf7, 0x1f, 0xbc,
    0x4e, 0x6f, 0x77, 0x50, 0xcb, 0x93, 0x87, 0x0e, 0xce, 0x70, 0xfc, 0xc5, 0x34, 0x8e, 0x56, 0xec,
    0xfd, 0x60, 0x99, 0x27, 0xff, 0x9a, 0xe6, 0xc8, 0xdf, 0xb2, 0x14, 0xca, 0x0e, 0x5b, 0xfb, 0x03,
    0xbe, 0xfd, 0xa5, 0x1c, 0x95, 0x4a, 0x05, 0xa5, 0x14, 0xf9, 0x42, 0x81, 0xa9, 0xe9, 0x69, 0x5e,
    0x1c, 0x3a, 0xc5, 0xaa, 0xd5, 0xab, 0x18, 0x5a, 0x70, 0xd9, 0xb9, 0x35, 0x11, 0xcb, 0x04, 0x5b,
    0xcd, 0x02, 0x5d, 0xdb, 0xfd, 0x1f, 0x9e, 0x15, 0xf2, 0x45, 0x8d, 0x56, 0x16, 0x41, 0x78, 0xe3,
    0x82, 0x43, 0xef, 0xb2, 0x4a, 0x8d, 0x40, 0xa6, 0xcd, 0x72, 0xef, 0xde, 0x1c, 0x4a, 0x29, 0x7e,
    0xf7, 0x4c, 0x96, 0xdf, 0xfe, 0x25, 0x03, 0x4a, 0x50, 0xca, 0x30, 0x72, 0xd1, 0xa1, 0xf3, 0x96,
    0x0a, 0x5d, 0xd9, 0x32, 0xb3, 0x39, 0xc8, 0xe7, 0xf2, 0x4c, 0xa8, 0x09, 0xca, 0xe5, 0x32, 0xda,
    0x71, 0xe8, 0xbe, 0xed, 0x13, 0x58, 0x0e, 0xa3, 0xb9, 0x5e, 0xcb, 0x02, 0x4d, 0x30, 0x09, 0xb6,
    0x08, 0x08, 0x65, 0x1f, 0x9e, 0xf9, 0x17, 0x88, 0x08, 0xc6, 0x1a, 0xac, 0xb5, 0x0c, 0x9f, 0x77,
    0xc3, 0xfc, 0x6d, 0x78, 0x0c, 0x67, 0xdf, 0x76, 0xf8, 0xcd, 0xd1, 0x76, 0x8c, 0x31, 0xb5, 0xe7,
    0xfd, 0xeb, 0x4a, 0x64, 0xda, 0x02, 0x36, 0xaf, 0x2f, 0x52, 0xa9, 0x54, 0x28, 0x96, 0x8a, 0xcc,
    0xcf, 0x2f, 0x30, 0x33, 0x73, 0x9d, 0xb1, 0x2b, 0x63, 0x8c, 0x8e, 0xcf, 0x73, 0xf1, 0x72, 0x29,
    0x8c, 0x39, 0x5b, 0x86, 0x60, 0x06, 0x17, 0x53, 0x08, 0xa3, 0x52, 0x2d, 0x30, 0x74, 0x3a, 0xcb,
    0xd4, 0xcc, 0x22, 0x32, 0xed, 0x4e, 0x2d, 0xe2, 0x87, 0xcf, 0xb9, 0x88, 0xd8, 0xba, 0x34, 0x00,
    0x56, 0xe0, 0x27, 0xbf, 0xcf, 0xe0, 0xfb, 0x16, 0xa5, 0xeb, 0xe7, 0xc2, 0xfe, 0xdd, 0x39, 0x10,
    0xc3, 0xd6, 0x5b, 0x0b, 0xfc, 0x7d, 0xa8, 0xad, 0x66, 0x69, 0x6b, 0x0d, 0x57, 0xc6, 0x7c, 0xdc,
    0x64, 0x82, 0xdc, 0xd8, 0x15, 0x36, 0x75, 0x4f, 0x82, 0xc9, 0x83, 0x54, 0x70, 0x43, 0x09, 0x08,
    0x45, 0xe8, 0xe4, 0x70, 0x81, 0xd9, 0xeb, 0x96, 0x39, 0x35, 0x47, 0x22, 0x99, 0x20, 0x95, 0x4c,
    0x51, 0x28, 0x24, 0x18, 0x9f, 0x30, 0xac, 0xec, 0xa9, 0x8b, 0xe3, 0x89, 0x57, 0xdb, 0x38, 0x7f,
    0xd9, 0x61, 0x69, 0x97, 0xcf, 0x03, 0x07, 0x0a, 0x3c, 0xfe, 0x54, 0x96, 0xdb, 0xfa, 0x03, 0xf6,
    0x0c, 0x16, 0x40, 0x14, 0xdb, 0x36, 0x94, 0xf0, 0xfd, 0x0a, 0x9e, 0xe7, 0x21, 0x14, 0x22, 0xdd,
    0x11, 0x8a, 0x85, 0x22, 0xb2, 0x4e, 0x83, 0xf8, 0x21, 0x33, 0x11, 0xdc, 0x9a, 0xec, 0x2a, 0xc5,
    0x7f, 0x46, 0xeb, 0xdb, 0xf4, 0x2b, 0x3e, 0x7e, 0xc5, 0x07, 0x60, 0xe8, 0xb5, 0x59, 0x3e, 0xf7,
    0xf1, 0x12, 0xe0, 0x20, 0xca, 0xe1, 0xb1, 0xc3, 0xdd, 0xcc, 0xcd, 0xcd, 0xf1, 0xe0, 0x81, 0x3c,
    0x9f, 0xd9, 0x53, 0x60, 0xff, 0x4e, 0x85, 0xe3, 0x5a, 0xb4, 0x11, 0xb0, 0xc2, 0xca, 0xf7, 0x19,
    0x8c, 0xe7, 0x33, 0x33, 0x97, 0xc0, 0x75, 0x13, 0xb8, 0x09, 0x17, 0xd7, 0x75, 0x99, 0x9d, 0x9d,
    0x65, 0x2e, 0xdf, 0x56, 0x57, 0x44, 0xa9, 0x47, 0x1f, 0x00, 0x53, 0x37, 0x9a, 0xcf, 0xce, 0xb0,
    0xbd, 0x74, 0x26, 0x19, 0x45, 0x6d, 0xc0, 0xdb, 0xef, 0x18, 0x5e, 0x39, 0x13, 0x10, 0xf8, 0x79,
    0x3e, 0xbd, 0x7b, 0x12, 0x4c, 0x8e, 0x84, 0xce, 0xa1, 0xa5, 0x00, 0x52, 0x02, 0xeb, 0xa1, 0x30,
    0xec, 0xd8, 0x52, 0x42, 0xac, 0xc5, 0x5a, 0x83, 0x09, 0x42, 0xfd, 0xa8, 0x78, 0x1e, 0xa5, 0xb2,
    0x8d, 0x49, 0xb2, 0xc4, 0x09, 0x28, 0x3c, 0xbf, 0x25, 0x3e, 0x2f, 0x8d, 0xa4, 0x6a, 0x3a, 0x75,
    0xec, 0x64, 0x3b, 0xa0, 0xd8, 0x3b, 0x58, 0x66, 0xf1, 0x22, 0x43, 0x4d, 0x47, 0x84, 0x68, 0xd1,
    0x30, 0x50, 0x07, 0xb7, 0x94, 0x10, 0xb1, 0x58, 0x63, 0xb0, 0xc6, 0x60, 0x4c, 0x80, 0x35, 0x01,
    0xca, 0x7a, 0x51, 0x20, 0x87, 0x24, 0x62, 0x04, 0x84, 0x6c, 0xba, 0x75, 0x6d, 0x70, 0x75, 0xca,
    0x61, 0xec, 0x9a, 0x03, 0xc0, 0xc9, 0xe1, 0x14, 0x00, 0xfb, 0x3e, 0x54, 0xaa, 0xe1, 0x56, 0x77,
    0x53, 0x3f, 0x79, 0x85, 0xc1, 0x81, 0x72, 0xcd, 0x02, 0xd6, 0x86, 0x24, 0xb4, 0xd6, 0xa4, 0x93,
    0xb9, 0x86, 0x8c, 0xd2, 0x55, 0x70, 0x44, 0x58, 0xd1, 0xdd, 0xda, 0x02, 0xa0, 0x78, 0x79, 0x24,
    0x45, 0x60, 0xe0, 0xad, 0x4b, 0xa1, 0x90, 0x1c, 0xfd, 0x47, 0x9a, 0x87, 0x7e, 0x75, 0x0b, 0x73,
    0x39, 0x55, 0x27, 0x11, 0xf3, 0x6d, 0xdf, 0xca, 0x0a, 0x8b, 0x17, 0x05, 0x20, 0x16, 0x89, 0x08,
    0x24, 0x5c, 0x87, 0x9e, 0xce, 0xb9, 0x66, 0x0b, 0xd4, 0x0b, 0x86, 0x6d, 0xfd, 0x82, 0xdc, 0x54,
    0xa0, 0x44, 0x6e, 0x38, 0x93, 0x64, 0x66, 0x56, 0x53, 0x2c, 0x87, 0x9c, 0x5f, 0x18, 0x6e, 0xe3,
    0xb1, 0x3f, 0x67, 0x39, 0xfc, 0x6c, 0x1a, 0x44, 0xa2, 0xd3, 0xbf, 0x4e, 0x42, 0x2b, 0x61, 0xfb,
    0xe6, 0x32, 0xd6, 0x4a, 0xf4, 0x58, 0xca, 0x5e, 0x99, 0x3b, 0xfa, 0x7d, 0xb0, 0xb6, 0x46, 0x42,
    0xd7, 0xcc, 0x87, 0x65, 0xcf, 0x9d, 0x60, 0xc4, 0x50, 0x5d, 0x2e, 0xde, 0x5e, 0x1e, 0x49, 0x61,
    0x5a, 0x70, 0x5b, 0xd7, 0x1b, 0xc9, 0xb4, 0x84, 0x67, 0x41, 0xed, 0x27, 0xc2, 0x07, 0xb6, 0x94,
    0xc1, 0x5a, 0xc4, 0x86, 0x60, 0xd6, 0x9f, 0x67, 0xe7, 0xed, 0xa1, 0x90, 0xdd, 0xec, 0x02, 0x2c,
    0x1b, 0xd7, 0x58, 0x06, 0xfa, 0x4c, 0xcb, 0x32, 0x71, 0x7c, 0xd2, 0xa5, 0xe2, 0xab, 0x28, 0xf0,
    0xc2, 0xb6, 0xa8, 0xc3, 0xb2, 0x6b, 0x47, 0xb9, 0x71, 0x62, 0x35, 0x18, 0x45, 0xd8, 0x31, 0x50,
    0x8a, 0xc8, 0x84, 0x42, 0xb6, 0xe7, 0xce, 0x39, 0x3a, 0xda, 0x4c, 0x44, 0xc0, 0x54, 0x09, 0xd4,
    0x23, 0x57, 0x61, 0xf8, 0xe6, 0x7d, 0x82, 0xb1, 0x01, 0x22, 0x37, 0xb3, 0x78, 0xf5, 0x6c, 0x8a,
    0xef, 0x7d, 0x79, 0x01, 0x15, 0x1d, 0xa5, 0x07, 0xef, 0xcf, 0xd1, 0x9e, 0x82, 0xd1, 0xab, 0x0e,
    0xcf, 0xbf, 0x92, 0x22, 0x57, 0x68, 0x4c, 0xe3, 0x4d, 0x6b, 0x7d, 0x6e, 0xc9, 0x04, 0x20, 0x82,
    0x31, 0x79, 0xbe, 0xfe, 0xd9, 0x20, 0x14, 0x21, 0x09, 0xa2, 0xf2, 0xcc, 0x46, 0x55, 0xb1, 0x48,
    0x58, 0x36, 0x61, 0xf8, 0xe4, 0x5d, 0x0e, 0xbb, 0xb6, 0x07, 0xfc, 0xfb, 0xf5, 0xba, 0x1c, 0xc7,
    0xe3, 0xe0, 0x67, 0xdf, 0x99, 0x63, 0xdb, 0xc6, 0x0a, 0xef, 0xbc, 0xe7, 0xf2, 0xb1, 0xbb, 0xca,
    0x3c, 0x79, 0x2c, 0xcd, 0x77, 0x1f, 0xe9, 0xc4, 0x8a, 0x62, 0xcd, 0xf2, 0x80, 0xe3, 0xbf, 0x9c,
    0xc2, 0x58, 0x18, 0x7a, 0xa3, 0x8d, 0x93, 0xa7, 0x53, 0x04, 0x46, 0x61, 0xac, 0xe1, 0x1b, 0xf7,
    0xf9, 0xac, 0xe9, 0x31, 0x35, 0xe0, 0xaa, 0x99, 0x95, 0xbc, 0x1e, 0xe9, 0x91, 0x4a, 0x81, 0x4e,
    0x81, 0x4a, 0x31, 0x3d, 0xef, 0xb2, 0xef, 0xab, 0x1e, 0xd7, 0xa6, 0x13, 0x61, 0xe5, 0x13, 0xb5,
    0x95, 0x3d, 0x01, 0xa7, 0xfe, 0x34, 0xd9, 0x70, 0x2e, 0xec, 0xfb, 0xca, 0x52, 0xce, 0xfd, 0x37,
    0x59, 0x1b, 0xaf, 0xe8, 0x0e, 0xb8, 0x36, 0xed, 0x62, 0x45, 0x45, 0x1e, 0x11, 0x76, 0x6d, 0xf7,
    0xf9, 0xe3, 0x0f, 0x13, 0x38, 0xca, 0x8f, 0xca, 0x32, 0x2f, 0x7c, 0x23, 0x68, 0x74, 0x07, 0x38,
    0x9d, 0xe0, 0x74, 0x84, 0x24, 0x94, 0x66, 0x69, 0xa7, 0xc3, 0xd1, 0x87, 0x3b, 0xe8, 0xed, 0x0e,
    0xb0, 0xd6, 0xd6, 0xdc, 0xf1, 0xee, 0x84, 0xc3, 0xf8, 0xa4, 0x83, 0x1f, 0xc0, 0xf0, 0xf9, 0x04,
    0xbf, 0x38, 0xdc, 0xc1, 0xf8, 0xa4, 0x7b, 0x53, 0xac, 0x18, 0x5b, 0x3f, 0x51, 0xef, 0xbe, 0xc3,
    0xf0, 0xeb, 0x1f, 0x64, 0x70, 0xb4, 0x26, 0x2c, 0xf7, 0x52, 0x84, 0x98, 0x5d, 0xa0, 0x33, 0x28,
    0x79, 0xb3, 0x47, 0xe2, 0xe7, 0x41, 0xad, 0x20, 0x55, 0x2e, 0x37, 0x72, 0x9a, 0x43, 0x3f, 0xce,
    0x73, 0xec, 0x64, 0x80, 0xab, 0x43, 0xa0, 0xcd, 0x7d, 0x3e, 0xef, 0x5e, 0x73, 0xc9, 0x15, 0x1b,
    0x54, 0xbc, 0x29, 0x0e, 0x05, 0xa5, 0x02, 0xbe, 0xf6, 0xf9, 0x14, 0xdf, 0xfa, 0x62, 0x1a, 0x57,
    0x07, 0x0d, 0x7e, 0xaf, 0x67, 0x9e, 0xa0, 0x64, 0x64, 0x75, 0xbc, 0x18, 0x8c, 0x5d, 0x3e, 0xaa,
    0xc5, 0xa9, 0xc3, 0x0b, 0xc3, 0x3e, 0x3f, 0x7f, 0x62, 0x81, 0x53, 0x6f, 0xfa, 0x28, 0x9c, 0x06,
    0xb7, 0x34, 0x00, 0x8b, 0xc5, 0x71, 0x0d, 0xf7, 0xdc, 0xdd, 0xc6, 0xa1, 0x2f, 0x64, 0xe9, 0x5f,
    0xa5, 0x81, 0x28, 0xe2, 0x89, 0x64, 0x3b, 0x06, 0x1e, 0xc6, 0xc0, 0xd9, 0x8d, 0xb1, 0x70, 0x8f,
    0xdf, 0xf9, 0x74, 0x53, 0x89, 0xae, 0x19, 0x9f, 0x34, 0x9c, 0x78, 0xad, 0xc4, 0xe9, 0xf3, 0x65,
    0x46, 0xaf, 0x06, 0xcc, 0xe7, 0xc2, 0xf4, 0x5a, 0xbc, 0xc8, 0x61, 0xfd, 0x4a, 0x97, 0xed, 0x03,
    0xed, 0xec, 0xde, 0xd1, 0x4e, 0x57, 0x56, 0x45, 0xa0, 0xb6, 0xfe, 0x6e, 0x00, 0x8f, 0x21, 0xca,
    0xb9, 0x6d, 0x4d, 0xf9, 0xd6, 0x7c, 0x09, 0xad, 0x92, 0x88, 0x5f, 0xcb, 0x54, 0xe3, 0xbc, 0xda,
    0x81, 0x54, 0x17, 0xb5, 0xd0, 0xdc, 0xd5, 0x7e, 0xf3, 0x35, 0xad, 0xde, 0x5c, 0x74, 0x7b, 0x0b,
    0x63, 0x36, 0x5f, 0xcd, 0x9a, 0x00, 0x9b, 0x6f, 0xc5, 0x0d, 0x8b, 0x57, 0x3d, 0x1a, 0x2f, 0xf5,
    0xe3, 0x04, 0x1a, 0x9b, 0x8b, 0x4e, 0xb7, 0x20, 0x10, 0x27, 0xd2, 0x44, 0x4a, 0x35, 0x7f, 0x8f,
    0x4f, 0x91, 0xc6, 0xdd, 0x36, 0x5c, 0x58, 0x5b, 0xb7, 0xff, 0x01, 0xf7, 0xb6, 0x2b, 0x6b, 0x6c,
    0xbf, 0xa4, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82} ;

const QString AlloCineScraper::ALLO_DEFAULT_URL_IMAGES="images.allocine.fr";


QString AlloCineScraper::getName() const{
    return "Allocine";
}

static QIcon m_icon;


QIcon AlloCineScraper::getIcon() const {
    if (m_icon.isNull()){
        QPixmap pixmap;
        if (pixmap.loadFromData(AlloCineScraper::icon_png, sizeof(AlloCineScraper::icon_png)/sizeof(uchar))){
            m_icon=QIcon(pixmap);
        }
    }

    return m_icon;
}

