#include "themoviedbscraper.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>

const QString TheMovieDBScraper::API_KEY="470fd2ec8853e25d2f8d86f685d2270e";

const uchar TheMovieDBScraper::icon_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff,
    0x61, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xa0,
    0xbd, 0xa7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00, 0x48, 0x00,
    0x00, 0x00, 0x48, 0x00, 0x46, 0xc9, 0x6b, 0x3e, 0x00, 0x00, 0x00, 0x09, 0x76, 0x70, 0x41, 0x67,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5c, 0xc6, 0xad, 0xc3, 0x00, 0x00, 0x02,
    0x78, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0xa5, 0x93, 0xcd, 0x6f, 0x94, 0x65, 0x14, 0xc5, 0x7f,
    0xf7, 0x9d, 0xe7, 0x7d, 0x67, 0x86, 0x04, 0xa4, 0x2d, 0xe5, 0xab, 0xd8, 0x92, 0x16, 0x0a, 0x21,
    0x04, 0x45, 0x31, 0xe1, 0xa3, 0x81, 0x26, 0x1a, 0xa3, 0x3b, 0x56, 0x2c, 0xdd, 0xf9, 0x4f, 0xb9,
    0x72, 0x4b, 0xc2, 0xc6, 0xc4, 0xa0, 0x89, 0x31, 0x60, 0x04, 0xfc, 0x80, 0x68, 0x4a, 0x53, 0xcc,
    0xa4, 0x54, 0x45, 0xea, 0x74, 0x28, 0x33, 0xc0, 0x54, 0xe6, 0xfd, 0x7c, 0x9e, 0x7b, 0x5d, 0x4c,
    0xd3, 0x88, 0xb8, 0x30, 0xe1, 0x24, 0x77, 0x71, 0x17, 0xe7, 0x24, 0xf7, 0x9c, 0x7b, 0x44, 0xcd,
    0x1b, 0xaf, 0x00, 0xf7, 0xcf, 0x45, 0x10, 0x40, 0xb6, 0xb6, 0x97, 0x61, 0x80, 0x61, 0xd8, 0x8b,
    0x02, 0x82, 0xa0, 0xaa, 0x6c, 0xa4, 0x7d, 0xba, 0xbd, 0x47, 0xf4, 0xfa, 0x5d, 0xd2, 0x2c, 0x45,
    0x83, 0xc7, 0xcc, 0x10, 0x89, 0xa8, 0xd7, 0x9b, 0x8c, 0xbc, 0x36, 0xc2, 0xf8, 0xd8, 0x1e, 0x46,
    0x76, 0x8c, 0xe1, 0x6a, 0x0e, 0xc3, 0x70, 0x82, 0xd0, 0xeb, 0x77, 0xb9, 0xbd, 0x70, 0x93, 0x5f,
    0x96, 0x17, 0x79, 0xfa, 0xac, 0x4b, 0x96, 0xa5, 0x54, 0xbe, 0x22, 0x84, 0x0a, 0x55, 0xc3, 0x0c,
    0x22, 0x22, 0x62, 0x17, 0xb3, 0x7d, 0xfb, 0x4e, 0x8e, 0x1d, 0x79, 0x93, 0xb9, 0xd3, 0xf3, 0x4c,
    0xec, 0x7e, 0x1d, 0x59, 0xef, 0xb5, 0xed, 0xb3, 0xaf, 0x2e, 0xd3, 0x5a, 0x59, 0x44, 0x43, 0x40,
    0xd5, 0x48, 0x5c, 0x9d, 0x38, 0x89, 0x11, 0xa9, 0x11, 0xd4, 0xf0, 0x65, 0x49, 0x96, 0x65, 0x64,
    0xd9, 0x80, 0xe0, 0x03, 0xaa, 0x30, 0x7b, 0xf8, 0x18, 0x1f, 0x5d, 0xfa, 0x18, 0xb9, 0xf2, 0xc5,
    0xa7, 0xf6, 0xe3, 0x4f, 0x37, 0x50, 0x55, 0x1a, 0x8d, 0x26, 0x27, 0x8e, 0xbe, 0xc5, 0xe1, 0xe9,
    0xe3, 0x8c, 0x8e, 0x8c, 0x11, 0xbb, 0x3a, 0x20, 0x94, 0x65, 0x41, 0xf7, 0xc9, 0x23, 0x96, 0x5a,
    0x77, 0xf9, 0xfe, 0xf6, 0xb7, 0x6c, 0xf4, 0xfb, 0xa8, 0x1a, 0xe7, 0xcf, 0xbd, 0x87, 0xbb, 0xb7,
    0xbc, 0x88, 0xe2, 0x31, 0x22, 0xde, 0x7e, 0x63, 0x8e, 0x0f, 0xe7, 0x2f, 0x92, 0xc4, 0x09, 0x6a,
    0x43, 0xc3, 0x44, 0x86, 0xd6, 0x4e, 0x4e, 0x1c, 0xe4, 0xf8, 0xd1, 0x93, 0x24, 0x49, 0x9d, 0xcf,
    0xaf, 0x5e, 0x21, 0x78, 0xcf, 0x9d, 0x9f, 0x7f, 0x20, 0x1a, 0x0c, 0xfe, 0xc2, 0x82, 0x50, 0x8b,
    0x6a, 0x4c, 0x4d, 0x4c, 0x6f, 0x92, 0x75, 0xd3, 0x71, 0x50, 0xd5, 0xe1, 0x98, 0x92, 0xc4, 0x09,
    0x33, 0x53, 0x47, 0x70, 0x51, 0x8c, 0x0f, 0x4a, 0xef, 0x49, 0x97, 0x48, 0x64, 0x98, 0x40, 0x08,
    0x81, 0xbc, 0xc8, 0xb7, 0xd2, 0x32, 0x1b, 0x0a, 0x88, 0xc8, 0x56, 0x68, 0x82, 0x90, 0xe7, 0x19,
    0x55, 0x50, 0x50, 0x23, 0x42, 0x88, 0xf6, 0x8c, 0xed, 0x43, 0x4d, 0x29, 0xab, 0x82, 0x3b, 0x0b,
    0xb7, 0x78, 0xd8, 0x7e, 0x80, 0x0f, 0x1e, 0x11, 0xd9, 0x22, 0x89, 0x08, 0x55, 0x55, 0xf1, 0xfb,
    0xea, 0xaf, 0x7c, 0x73, 0xeb, 0x6b, 0xd2, 0x2c, 0xa5, 0xf4, 0x9e, 0x89, 0xfd, 0x93, 0xb8, 0x77,
    0x4e, 0xce, 0xb1, 0xb6, 0xde, 0x66, 0x30, 0xd8, 0xa0, 0xb5, 0x7c, 0x97, 0x4f, 0xd6, 0xd7, 0x98,
    0x3c, 0x30, 0xc3, 0xe8, 0xce, 0x5d, 0xc4, 0x2e, 0xc6, 0xd4, 0x28, 0xf2, 0x9c, 0x4e, 0xb7, 0xc3,
    0xfd, 0x95, 0x16, 0x9d, 0xb5, 0x55, 0xd4, 0x2b, 0x8d, 0x7a, 0x83, 0x77, 0x2f, 0x7c, 0x80, 0x3b,
    0x75, 0xe2, 0x2c, 0x69, 0x9a, 0x72, 0xed, 0xe6, 0x97, 0x3c, 0x7d, 0xd6, 0xa5, 0xd3, 0x59, 0xa5,
    0xbd, 0xfa, 0x07, 0xb6, 0x79, 0x7f, 0xf0, 0x81, 0xaa, 0xf2, 0xf8, 0x2a, 0x10, 0x42, 0x20, 0x98,
    0x11, 0x7c, 0x60, 0xf7, 0xf8, 0x5e, 0x4e, 0x9f, 0x3a, 0x83, 0x8b, 0xe3, 0x84, 0xf9, 0x33, 0xef,
    0x33, 0x33, 0x35, 0xcb, 0x52, 0x6b, 0x81, 0xdf, 0x1e, 0xae, 0xf0, 0xb8, 0xb7, 0x4e, 0xf6, 0x7c,
    0x40, 0x5e, 0x14, 0x68, 0x50, 0x00, 0xea, 0x8d, 0x6d, 0xec, 0x1a, 0x1d, 0xe7, 0xf9, 0x60, 0xc0,
    0xd2, 0xd2, 0x02, 0x65, 0x5e, 0x20, 0x22, 0x38, 0x01, 0x9c, 0x73, 0x4c, 0x4f, 0xce, 0x72, 0xf0,
    0xc0, 0x21, 0xf2, 0x2a, 0xa3, 0x2c, 0x8b, 0xe1, 0xe3, 0xe4, 0x19, 0x00, 0x49, 0x9c, 0xd0, 0x6c,
    0x6e, 0xa3, 0xd1, 0x68, 0x52, 0x14, 0x39, 0x37, 0xbe, 0xbb, 0x4e, 0xbb, 0xfd, 0x27, 0x49, 0xd2,
    0x40, 0xfe, 0xdd, 0x46, 0xf9, 0xcf, 0x12, 0xc9, 0x0b, 0x25, 0x32, 0x03, 0x1f, 0x2a, 0x5c, 0xcd,
    0xbd, 0x2c, 0xf0, 0x7f, 0x21, 0x08, 0x86, 0xf1, 0x37, 0x6e, 0x0b, 0x55, 0x92, 0x47, 0xd6, 0x6d,
    0xef, 0x00, 0x00, 0x00, 0x25, 0x74, 0x45, 0x58, 0x74, 0x64, 0x61, 0x74, 0x65, 0x3a, 0x63, 0x72,
    0x65, 0x61, 0x74, 0x65, 0x00, 0x32, 0x30, 0x31, 0x31, 0x2d, 0x30, 0x33, 0x2d, 0x32, 0x30, 0x54,
    0x31, 0x34, 0x3a, 0x30, 0x38, 0x3a, 0x35, 0x33, 0x2d, 0x30, 0x34, 0x3a, 0x30, 0x30, 0x74, 0x69,
    0x86, 0xc6, 0x00, 0x00, 0x00, 0x25, 0x74, 0x45, 0x58, 0x74, 0x64, 0x61, 0x74, 0x65, 0x3a, 0x6d,
    0x6f, 0x64, 0x69, 0x66, 0x79, 0x00, 0x32, 0x30, 0x31, 0x31, 0x2d, 0x30, 0x33, 0x2d, 0x32, 0x30,
    0x54, 0x31, 0x34, 0x3a, 0x30, 0x38, 0x3a, 0x35, 0x33, 0x2d, 0x30, 0x34, 0x3a, 0x30, 0x30, 0x05,
    0x34, 0x3e, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82} ;



TheMovieDBScraper::TheMovieDBScraper()
{
}

QIcon TheMovieDBScraper::getIcon() {
    if (m_icon.isNull()){
        QPixmap pixmap;
        if (pixmap.loadFromData(TheMovieDBScraper::icon_png, sizeof(TheMovieDBScraper::icon_png)/sizeof(uchar))){
            m_icon=QIcon(pixmap);
        }
    }

    return m_icon;
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

    if (movieObject["credits"].isObject()){
        QJsonObject creditsObject=movieObject["credits"].toObject();
        if (creditsObject["cast"].isArray()){
            QJsonArray jsonArray = creditsObject["cast"].toArray();

            foreach (const QJsonValue & value, jsonArray)
            {
                if (value.isObject() &&  value.toObject()["name"].isString()){
                    info.actors.append(value.toObject()["name"].toString());
                }
            }
        }

        if (creditsObject["crew"].isArray()){
            QJsonArray jsonArray = creditsObject["crew"].toArray();

            foreach (const QJsonValue & value, jsonArray)
            {
                if (value.isObject() &&  value.toObject()["job"].isString() && value.toObject()["job"].toString().compare("Director",Qt::CaseInsensitive)==0 &&  value.toObject()["name"].isString()){
                    info.directors.append(value.toObject()["name"].toString());
                }
            }
        }
    }

    info.productionYear=0;
    if (movieObject["release_date"].isString() ){
        QDate releaseDate=QDate::fromString(movieObject["release_date"].toString(), "yyyy-MM-dd");
        if (releaseDate.isValid()){
            info.productionYear=releaseDate.year();
        }
    }

    info.runtime=movieObject["runtime"].toInt()*60;

    return true;
}

bool TheMovieDBScraper::findMovieInfo( const QString& movieCode, SearchMovieInfo& result) const {

    QMap<QString,QString> params;
    QByteArray headerData;
    QByteArray data;

    params["language"]="fr";
    params["append_to_response"]="credits";

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
