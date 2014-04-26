#include "themoviedbscraper.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>

//#include <curl/curl.h>

const QString TheMovieDBScraper::API_KEY="470fd2ec8853e25d2f8d86f685d2270e";

TheMovieDBScraper::TheMovieDBScraper()
{
}

QString TheMovieDBScraper::createURL(const QString& type, const QMap<QString, QString> &params) const
{
    QString queryURL=QString().append("http://api.themoviedb.org/3/").append(type);

    QString mapParams;
    foreach( QString key, params.keys() )
    {
        mapParams.append(QUrl::toPercentEncoding(key)).append('=').append(params.value( QUrl::toPercentEncoding(key) )).append('&');
    }

    QString searchQuery = mapParams;

    QString fullQuery= queryURL.append('?').append(searchQuery).append("api_key=").append(QUrl::toPercentEncoding(API_KEY));

    qDebug() << fullQuery;

    return fullQuery;
}

 bool TheMovieDBScraper::searchTV(const QString& toSearch, SearchTVResult &result){
     QMap<QString,QString> params;

     QByteArray headerData;
     QByteArray data;

     if (execCommand("configuration", params, headerData,data)){
         qDebug() << data;
         QJsonParseError e;
         QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
         if (e.error== QJsonParseError::NoError){
             if (!parseConfiguration(doc)){
                 return false;
             }
         } else {
             return false;
         }
     } else {
         return false;
     }

     headerData.clear();
     data.clear();
     params["query"]=QUrl::toPercentEncoding(toSearch);
     params["language"]="fr";

     if (execCommand("search/tv", params, headerData,data)){
         qDebug() << data;

         QJsonParseError e;
         QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
         if (e.error== QJsonParseError::NoError){
             result.searchOk = true;
             result.shows=this->parseTVResultset(doc);
             return true;
         }

         qDebug() << e.errorString();

     }

     return false;
 }


bool TheMovieDBScraper::searchFilm( const QString& toSearch, SearchResult& result) {


    QMap<QString,QString> params;

    QByteArray headerData;
    QByteArray data;

    if (execCommand("configuration", params, headerData,data)){
        qDebug() << data;
        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if (!parseConfiguration(doc)){
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }

    headerData.clear();
    data.clear();
    params["query"]=QUrl::toPercentEncoding(toSearch);
    params["language"]="fr";

    if (execCommand("search/movie", params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            result.searchOk = true;
            result.films=parseResultset(doc);
            return true;
        }

        qDebug() << e.errorString();

    }

    return false;
}

bool TheMovieDBScraper::parseConfiguration(const QJsonDocument& resultset) {
    if (!resultset.isObject()){
        return false;
    }

    QJsonObject jsonObject = resultset.object();

    if (!jsonObject["images"].isObject()){
        return false;
    }

    QJsonObject imagesObject = jsonObject["images"].toObject();

    baseUrl = imagesObject["base_url"].toString();

    if (!imagesObject["poster_sizes"].isArray()){
        return false;
    }

    foreach (const QJsonValue & value, imagesObject["poster_sizes"].toArray())
    {
        posterSizes << value.toString();
    }

    if (!imagesObject["backdrop_sizes"].isArray()){
        return false;
    }

    foreach (const QJsonValue & value, imagesObject["backdrop_sizes"].toArray())
    {
        backdropSizes << value.toString();
    }

    return true;
}

QString findBestSize(const QStringList& listSizes, int sizeToFit){

    QRegExp sizeRegExp ("[wh](\\d+)");
    int delta=0;
    QString choosedSized="original";

    foreach (const QString& size, listSizes)
    {
        qDebug() << size;
        if (sizeRegExp.exactMatch(size) && sizeRegExp.captureCount()==1) {
            int curSize=sizeRegExp.cap(1).toInt();
            if (delta==0 || qAbs(curSize-sizeToFit)<delta){
                delta=qAbs(curSize-sizeToFit);
                if (delta==0){
                    return size;
                } else {
                    choosedSized=size;
                }
            }
        }
    }


    return choosedSized;
}

FilmPrtList TheMovieDBScraper::parseResultset(const QJsonDocument& resultset) const{

    FilmPrtList films;

    if (!resultset.isObject()){
        return films;
    }

    QJsonObject jsonObject = resultset.object();


    if (!jsonObject["results"].isArray()){
        return films;
    }

    QJsonArray jsonArray = jsonObject["results"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        FilmPtr film(new Film());

        film->originalTitle= obj["original_title"].toString();
        film->title= obj["title"].toString();
        if (film->title.isEmpty()){
            film->title=film->originalTitle;
        }
        film->productionYear =QDate::fromString(obj["release_date"].toString(), "yyyy-MM-dd").toString("yyyy");
        film->code= QString::number(obj["id"].toDouble());
        film->posterHref = QString().append(baseUrl).append("original").append(obj["poster_path"].toString());

        films.append(film);
    }

    return films;
}

QList<Show> TheMovieDBScraper::parseTVResultset(const QJsonDocument& resultset) const{

    QList<Show> shows;

    if (!resultset.isObject()){
        return shows;
    }

    QJsonObject jsonObject = resultset.object();


    if (!jsonObject["results"].isArray()){
        return shows;
    }

    QJsonArray jsonArray = jsonObject["results"].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        Show show;
        show.originalTitle= obj["original_name"].toString();
        show.title= obj["name"].toString();
        show.productionYear =QDate::fromString(obj["first_air_date"].toString(), "yyyy-MM-dd").toString("yyyy");
        show.code= QString::number(obj["id"].toDouble());
        show.posterHref = QString().append(baseUrl).append("original").append(obj["poster_path"].toString());

        shows.append(show);
    }

    return shows;
}

bool TheMovieDBScraper::parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const{
    if (!resultset.isObject()){
        return false;
    }
    QJsonObject movieObject = resultset.object();


    info.synopsis=movieObject["overview"].toString();
    info.posterHref =   QString().append(baseUrl).append("original").append(movieObject["poster_path"].toString());
    info.backdropHref =   QString().append(baseUrl).append("original").append(movieObject["backdrop_path"].toString());
    info.postersHref.append(movieObject["poster_path"].toString());
    info.backdropsHref.append(movieObject["backdrop_path"].toString());

    return true;
}
bool TheMovieDBScraper::findMovieInfo( const QString& movieCode, SearchMovieInfo& result) const {

    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    params["language"]="fr";

    if (execCommand(QString("movie/").append(movieCode), params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if(parseMovieInfo(doc,result)){
                return getImage(movieCode, result);
            }
        }

        qDebug() << e.errorString();
    }

    return false;
}

bool TheMovieDBScraper::findTVInfo(const QString& showCode, SearchEpisodeInfo &result) const{

    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    params["language"]="fr";

    if (execCommand(QString("tv/").append(showCode), params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            /*if(parseMovieInfo(doc,result)){
                return getImage(movieCode, result);
            }*/
        }

        qDebug() << e.errorString();
    }

    return false;
}

bool TheMovieDBScraper::findSaisonInfo(const QString& showCode, const QString& season, SearchEpisodeInfo &result) const{

    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    params["language"]="fr";

    if (execCommand(QString("tv/").append(showCode).append("/season/").append(season), params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            /*if(parseMovieInfo(doc,result)){
                return getImage(movieCode, result);
            }*/
        }

        qDebug() << e.errorString();
    }

    findTVInfo(showCode,result);

    return false;
}

bool TheMovieDBScraper::findEpisodeInfo(const QString& showCode, const QString& season, const QString& episode, SearchEpisodeInfo &result) const {

    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    params["language"]="fr";

    if (execCommand(QString("tv/").append(showCode).append("/season/").append(season).append("/episode/").append(episode), params, headerData,data)){
        qDebug() << data;

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            /*if(parseMovieInfo(doc,result)){
                return getImage(movieCode, result);
            }*/
        }

        qDebug() << e.errorString();
    }

    findSaisonInfo(showCode, season, result);

    return false;
}

bool TheMovieDBScraper::parseImageInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const{

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject movieObject = resultset.object();

    if (!movieObject["backdrops"].isArray()){
        return false;
    }

    QJsonArray backdropsArray = movieObject["backdrops"].toArray();

    foreach (const QJsonValue & value, backdropsArray)
    {
        QJsonObject backdropObject = value.toObject();

        info.backdropsHref.append( backdropObject["file_path"].toString());
    }
    QJsonArray postersArray = movieObject["posters"].toArray();

    foreach (const QJsonValue & value, postersArray)
    {
        QJsonObject posterObject = value.toObject();

        info.postersHref.append( posterObject["file_path"].toString());
    }

    return true;
}

bool TheMovieDBScraper::getImage(const QString& movieCode,  SearchMovieInfo& result) const{
    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    if (execCommand(QString("movie/").append(movieCode).append("/images"), params, headerData,data)){

        QJsonParseError e;
        QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
        if (e.error== QJsonParseError::NoError){
            if(parseImageInfo(doc,result)){
                return true;
            }
        }

        qDebug() << e.errorString();
    }

    return false;

}

QString TheMovieDBScraper::getBestImageUrl(const QString& filePath, const QSize& size) const {
    return QString().append(baseUrl).append(findBestSize(posterSizes,size.width())).append(filePath);
}
