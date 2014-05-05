#include "allocinescraper.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>

//#include <curl/curl.h>

const QString AlloCineScraper::ALLOCINE_SECRET_KEY="29d185d98c984a359e6e6f26a0474269";
const QString AlloCineScraper::ALLO_DEFAULT_URL_API="api.allocine.fr";
const QString AlloCineScraper::ALLO_DEFAULT_URL_IMAGES="images.allocine.fr";
const QString AlloCineScraper::ALLO_PARTNER="100043982026";

AlloCineScraper::AlloCineScraper()
{
}

QString AlloCineScraper::createURL(const QString& type, const QMap<QString, QString> &params) const
{
    QString queryURL=QString().append(ALLO_DEFAULT_URL_API).append('/').append(type);

    QString mapParams;
    foreach( QString key, params.keys() )
    {
        mapParams.append(key).append('=').append(params.value( key )).append('&');
    }

    QString searchQuery = QString("partner=").append(ALLO_PARTNER).append("&format=json&").append(mapParams).append("sed=").append( QDate::currentDate().toString("yyyyMMdd"));

    qDebug() << searchQuery;

    QString toEncrypt = ALLOCINE_SECRET_KEY+searchQuery;


    QString fullQuery= queryURL.append('?').append(searchQuery).append("&sig=").append(QUrl::toPercentEncoding(QCryptographicHash::hash(toEncrypt.toUtf8(), QCryptographicHash::Sha1).toBase64()));

    qDebug() << fullQuery;

    return fullQuery;
}

bool AlloCineScraper::searchFilm(const QString& toSearch, SearchResult &result)  {
    QMap<QString,QString> params;
    params["filter"]=QUrl::toPercentEncoding("movie");
    params["q"]=QUrl::toPercentEncoding(toSearch);

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/search", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            result.films=parseResultset(doc);
            return true;
        }

        qDebug() << e.errorString();

    }

    return false;
}
bool AlloCineScraper::searchTV(const QString& toSearch, SearchTVResult &result)  {
    QMap<QString,QString> params;
    params["filter"]="tvseries";
    params["q"]=QUrl::toPercentEncoding(toSearch);

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/search", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            result.shows=parseTVResultset(doc);
            return true;
        }

        qDebug() << e.errorString();

    }

    return false;
}
bool AlloCineScraper::findMovieInfo( const QString& movieCode, SearchMovieInfo& result) const {

    QMap<QString,QString> params;
    params["filter"]=QUrl::toPercentEncoding("movie");
    params["code"]=QUrl::toPercentEncoding(movieCode);
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/movie", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if(parseMovieInfo(doc,result)){
                return true;
            }
        }

        qDebug() << e.errorString();
    }

    return false;
}


bool AlloCineScraper::parseLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const QString&  episode, QString& episodeCode) const{

    bool bOk;
    int episodeToFind = episode.toInt(&bOk);
    if (!bOk){
        return false;
    }

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["season"].isObject()){
        return false;
    }

    QJsonObject seasonObject = jsonObject["season"].toObject();


    if (!seasonObject["episode"].isArray()){
        return false;
    }

    QJsonArray jsonArray = seasonObject["episode"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject season = value.toObject();
        if(season["episodeNumberSeason"].toInt()==episodeToFind){
            episodeCode.setNum(season["code"].toInt());
            return true;
        }
    }

    return false;
}

bool AlloCineScraper::parseLargeTVSerieInfo(const QJsonDocument& resultset, const QString&  season, QString& seasonCode) const{

    bool bOk;
    int seasonToFind = season.toInt(&bOk);
    if (!bOk){
        return false;
    }

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
            return true;
        }
    }

    return false;
}

bool AlloCineScraper::findSaisonInfo(const QString& seasonCode, const QString& episode, QString& episodeCode) const{
    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(seasonCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/season", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if(parseLargeSeasonTVSerieInfo(doc,episode,episodeCode)){
                return true;
            }
        }

        qDebug() << e.errorString();
    }

    return false;
}

bool parseEpisodeTVSerieInfo(const QJsonDocument& resultset, SearchEpisodeInfo &result){

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
    result.title=episodeObject["title"].toString();
    result.originalTitle=episodeObject["title"].toString();

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

bool AlloCineScraper::findEpisodeInfo(const QString& episodeCode, SearchEpisodeInfo &result) const{
    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(episodeCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/episode", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if(parseEpisodeTVSerieInfo(doc,result)){
                return true;
            }
        }

        qDebug() << e.errorString();
    }

    return false;
}
bool AlloCineScraper::findEpisodeInfo(const QString& showCode, const QString&  season, const QString& epidode, SearchEpisodeInfo &result) const{
    QMap<QString,QString> params;
    params["code"]=QUrl::toPercentEncoding(showCode);
    params["profile"]=QUrl::toPercentEncoding("large");
    params["striptags"]=QUrl::toPercentEncoding("synopsis,synopsisshort");

    QByteArray headerData;
    QByteArray data;

    if (execCommand("rest/v3/tvseries", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            QString seasonCode;
            if(parseLargeTVSerieInfo(doc,season,seasonCode)){
                QString episodeCode;
                if(findSaisonInfo(seasonCode, epidode,episodeCode)){
                    if (findEpisodeInfo(episodeCode,result)){
                        return true;
                    }
                }
            }
        }

        qDebug() << e.errorString();
    }

    return false;
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
        info.actors=movieObject["castingShort"].toObject()["actors"].toString().split(",", QString::SkipEmptyParts);
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

QString AlloCineScraper::getBestImageUrl(const QString& filePath, const QSize& size) const{
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

QList<Show> AlloCineScraper::parseTVResultset(const QJsonDocument& resultset) const{

    QList<Show> shows;

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

        Show show;
        show.originalTitle= obj["originalTitle"].toString();
        show.title= obj["originalTitle"].toString();
        show.productionYear = QString::number(obj["yearStart"].toDouble());
        show.code= QString::number(obj["code"].toDouble());
        if(obj["poster"].isObject()){
            show.posterHref = obj["poster"].toObject()["href"].toString();
        }

        shows.append(show);
    }

    return shows;
}
