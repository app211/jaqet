
#include "allocinescraper.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QNetworkReply>
#include <QApplication>
#include <QFile>

#include "../promise.h"
#include "../utils.h"
#include "../jsonhelper.h"

const QString AlloCineScraper::ALLO_DEFAULT_URL_IMAGES=QStringLiteral("images.allocine.fr");

QMap<int,QString> AlloCineScraper::codesToCountries;

namespace  {
QString CODE(QStringLiteral("code"));
QString PROFILE(QStringLiteral("profile"));
QString DOLLAR(QStringLiteral("$"));
QString HREF(QStringLiteral("href"));
QString MOVIE(QStringLiteral("movie"));
QString FILTER(QStringLiteral("filter"));
QString Q(QStringLiteral("q"));
QString TITLE(QStringLiteral("title"));
QString ORIGINALTITLE(QStringLiteral("originalTitle"));
QString TVSERIES(QStringLiteral("tvseries"));
QString RUNTIME(QStringLiteral("runtime"));

QString LARGE(QStringLiteral("large"));
QString MEDIUM(QStringLiteral("medium"));

QString ORIGINALCHANNEL(QStringLiteral("originalChannel"));
QString GENRE(QStringLiteral("genre"));
QString EPISODE(QStringLiteral("episode"));
QString STRIPTAGS(QStringLiteral("striptags"));
QString STRIPSYNOPSIS(QStringLiteral("synopsis,synopsisshort"));

QString URL_MOVIE(QStringLiteral("rest/v3/movie"));
QString URL_TVSERIES(QStringLiteral("rest/v3/tvseries"));
QString URL_SEARCH(QStringLiteral("rest/v3/search"));
QString URL_EPISODE(QStringLiteral("rest/v3/episode"));
QString URL_MEDIA(QStringLiteral("rest/v3/media"));
}

AlloCineScraper::AlloCineScraper(QObject *parent)
    :Scraper(parent), m_icon(loadIcon())
{
    init();
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

    QString fullQuery= QStringLiteral("http://api.allocine.fr/%2?%3&sig=%4").arg( type, searchQuery, QUrl::toPercentEncoding(QCryptographicHash::hash(toEncrypt.toUtf8(), QCryptographicHash::Sha1).toBase64()));

    qDebug().noquote() << fullQuery;

    return fullQuery;
}

void AlloCineScraper::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString&, int year) const
{
    Q_UNUSED(year)

    QMap<QString,QString> params;
    params[FILTER]=QUrl::toPercentEncoding(MOVIE);
    params[Q]=QUrl::toPercentEncoding(toSearch);

    QString url=createURL(URL_SEARCH,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "internalSearchFilm:" << data;
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                emit found(parseResultset(doc));
            } else {
                emit scraperError();
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

template<typename T>
bool extractGenre(const QJsonObject mediaObject,  T& mediaSearchPtr ){
    if (mediaObject[GENRE].isArray()){
        QStringList genres;

        QJsonArray jsonArrayGenre = mediaObject[GENRE].toArray();

        foreach (const QJsonValue & value, jsonArrayGenre)
        {
            QString genre = value.toObject()[DOLLAR].toString();
            if (!genre.isEmpty()) genres.append(genre);
        }

        mediaSearchPtr->setGenre(genres);
    }

    return true;
}

void AlloCineScraper::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const {

    Q_UNUSED(language);

    QMap<QString,QString> params;
    params[FILTER]=TVSERIES;
    params[Q]=QUrl::toPercentEncoding(toSearch);

    QString url=createURL(URL_SEARCH,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "internalSearchTV:" << data;

            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                emit found(parseTVResultset(doc));
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

void  AlloCineScraper::internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) {

    Q_UNUSED(language);

    QString movieCode= mediaMovieSearchPtr->foundResult().getCode();

    QMap<QString,QString> params;
    params[FILTER]=QUrl::toPercentEncoding(MOVIE);
    params[CODE]=QUrl::toPercentEncoding(movieCode);
    params[STRIPTAGS]=QUrl::toPercentEncoding(STRIPSYNOPSIS);

    if (searchFor & SearchOption::AllMedia){
        params[PROFILE]=QUrl::toPercentEncoding(LARGE);
    } else {
        params[PROFILE]=QUrl::toPercentEncoding(MEDIUM);
    }

    QString url=createURL(URL_MOVIE,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "internalFindMovieInfo:" << data;
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                if(parseMovieInfo(manager,doc,searchFor,mediaMovieSearchPtr)){
                    emit found(this, mediaMovieSearchPtr);
                } else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

void AlloCineScraper::internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor &searchFor, const QString &language)  {

    Q_UNUSED(searchFor);
    Q_UNUSED(language);

    QString showCode= mediaTVSearchPtr->foundResult().getCode();
    int season=mediaTVSearchPtr->foundResult().getSeason();
    int episode=mediaTVSearchPtr->foundResult().getEpisode();

    QMap<QString,QString> params;
    params[CODE]=QUrl::toPercentEncoding(showCode);
    params[PROFILE]=QUrl::toPercentEncoding(LARGE); // large for season id
    params[STRIPTAGS]=QUrl::toPercentEncoding(STRIPSYNOPSIS);

    QString url=createURL(URL_TVSERIES,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "internalFindEpisodeInfo:" << data;

             QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                QString seasonCode;
                if (searchFor & SearchOption::Information){
                    mediaTVSearchPtr->setEpisode(episode);
                    mediaTVSearchPtr->setSeason(season);
                }

                if(extractSeasonCodeFromLargeTVSerieInfo(doc,season,seasonCode,mediaTVSearchPtr) && !seasonCode.isEmpty()){
                    findSeasonInfoByCode(manager, seasonCode, episode,mediaTVSearchPtr);
                } else {
                    emit scraperError(tr("Unable to retrieve season code"));
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

bool AlloCineScraper::extractSeasonCodeFromLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString& seasonCode, MediaTVSearchPtr mediaTVSearchPtr) const{

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject[TVSERIES].isObject()){
        return false;
    }

    QJsonObject tvseriesObject = jsonObject[TVSERIES].toObject();

    mediaTVSearchPtr->setTitle(tvseriesObject[TITLE].toString());
    mediaTVSearchPtr->setOriginalTitle(tvseriesObject[ORIGINALTITLE].toString());

    if (tvseriesObject[ORIGINALCHANNEL].isObject() && tvseriesObject[ORIGINALCHANNEL].toObject()[QStringLiteral("channel")].isObject()){
        mediaTVSearchPtr->addNetwork(tvseriesObject[ORIGINALCHANNEL].toObject()[QStringLiteral("channel")].toObject()[QStringLiteral("name")].toString());
    }

    if (tvseriesObject[QStringLiteral("media")].isArray()){
        parseMedia(tvseriesObject[QStringLiteral("media")].toArray(), Scraper::SearchOption::All, mediaTVSearchPtr);
    }

    mediaTVSearchPtr->setProductionYear(tvseriesObject[QStringLiteral("yearStart")].toInt());

    if (tvseriesObject[QStringLiteral("statistics")].isObject()){
        double rating;
        if (extractRating(tvseriesObject[QStringLiteral("statistics")].toObject(),rating)){
            mediaTVSearchPtr->setShowRating(rating);
        }
    }

    extractGenre<MediaTVSearchPtr>(tvseriesObject,mediaTVSearchPtr);

    for (const QJsonValue & value : tvseriesObject[QStringLiteral("season")].toArray())
    {
        QJsonObject season = value.toObject();
        if(season[QStringLiteral("seasonNumber")].toInt()==seasonToFind){
            seasonCode.setNum(season[CODE].toInt());
            return true;
        }
    }

    return false;
}

void AlloCineScraper::findSeasonInfoByCode(QNetworkAccessManager *manager, const QString seasonCode, const int episode, MediaTVSearchPtr mediaTVSearchPtr) const{

    QMap<QString,QString> params;
    params[CODE]=QUrl::toPercentEncoding(seasonCode);
    params[PROFILE]=QUrl::toPercentEncoding(LARGE);
    params[STRIPTAGS]=QUrl::toPercentEncoding(STRIPSYNOPSIS);

    QString url=createURL(QStringLiteral("rest/v3/season"),params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "findSeasonInfoByCode:" << data;

            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                QString episodeCode;
                if(extractEpisodeCodeFromLargeSeasonTVSerieInfo(doc,episode,episodeCode, mediaTVSearchPtr)){
                    findEpisodeInfoByCode(manager,episodeCode, mediaTVSearchPtr);
                } else {
                    emit scraperError(tr("Unable to retrieve episode code"));

                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

bool AlloCineScraper::extractEpisodeCodeFromLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode, MediaTVSearchPtr mediaTVSearchPtr) const{


    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject[QStringLiteral("season")].isObject()){
        return false;
    }

    QJsonObject season = jsonObject[QStringLiteral("season")].toObject();

    if (season[QStringLiteral("media")].isArray()){
        parseMedia(season[QStringLiteral("media")].toArray(), Scraper::SearchOption::All, mediaTVSearchPtr);
    }

    if (season[ORIGINALCHANNEL].isObject() && season[ORIGINALCHANNEL].toObject()[QStringLiteral("channel")].isObject()){
        mediaTVSearchPtr->addNetwork(season[ORIGINALCHANNEL].toObject()[QStringLiteral("channel")].toObject()[QStringLiteral("name")].toString());
    }

    if (!season[EPISODE].isArray()){
        return false;
    }

    QJsonArray episodes = season[EPISODE].toArray();

    foreach (const QJsonValue & value, episodes)
    {
        QJsonObject episode = value.toObject();
        if(episode[QStringLiteral("episodeNumberSeason")].toInt()==episodeToFind ){
            episodeCode.setNum(episode[CODE].toInt());
            mediaTVSearchPtr->setProductionYear(season[QStringLiteral("yearStart")].toInt());
            return true;
        }
    }

    return false;
}

template<typename T>
bool parseMediaTemplate(const QJsonArray& mediaArray,const Scraper::SearchFor& searchFor,  T mediaSearchPtr ){

    if (searchFor & Scraper::SearchOption::AllMedia){
        foreach (const QJsonValue & value, mediaArray)
        {
            QJsonObject media = value.toObject();
            if (media[QStringLiteral("class")]==QStringLiteral("picture")){
                if (media[QStringLiteral("type")].isObject()){
                    QJsonObject typeObject = media[QStringLiteral("type")].toObject();
                    if (typeObject[DOLLAR].isString() && (typeObject[DOLLAR].toString()==QStringLiteral("Affiche") || typeObject[DOLLAR].toString()==QStringLiteral("Photo"))){
                        QJsonObject thumbnailObject=media[QStringLiteral("thumbnail")].toObject();
                        if (thumbnailObject[HREF].isString()){
                            if (typeObject[DOLLAR].toString()==QStringLiteral("Affiche")){
                                if (searchFor & Scraper::SearchOption::Poster){
                                    mediaSearchPtr->addPoster(thumbnailObject[HREF].toString(),QSize(media[QStringLiteral("width")].toInt(), media[QStringLiteral("height")].toInt()));
                                }
                            } else {
                                if (searchFor & Scraper::SearchOption::BackDrop){
                                    mediaSearchPtr->addBackdrop(thumbnailObject[HREF].toString(),QSize(media[QStringLiteral("width")].toInt(), media[QStringLiteral("height")].toInt()));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}



bool AlloCineScraper::parseMedia(const QJsonArray& mediaArray,const Scraper::SearchFor& searchFor,  MediaTVSearchPtr mediaTVSearchPtr ) const {
    return parseMediaTemplate<MediaTVSearchPtr>(mediaArray,searchFor,mediaTVSearchPtr);
}

bool AlloCineScraper::parseMedia(const QJsonArray& mediaArray,const Scraper::SearchFor& searchFor,  MediaMovieSearchPtr mediaMovieSearchPtr ) const {
    return parseMediaTemplate<MediaMovieSearchPtr>(mediaArray,searchFor,mediaMovieSearchPtr);
}

bool AlloCineScraper::parseEpisodeTVSerieInfo(const QJsonDocument& resultset, MediaTVSearchPtr mediaTVSearchPtr) const {

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject[EPISODE].isObject()){
        return false;
    }

    QJsonObject episodeObject = jsonObject[EPISODE].toObject();

    mediaTVSearchPtr->setSynopsis(episodeObject[QStringLiteral("synopsis")].toString());
    mediaTVSearchPtr->setCode(episodeObject[CODE].toString());
    mediaTVSearchPtr->setEpisodeTitle(episodeObject[TITLE].toString());
    mediaTVSearchPtr->setOriginalEpisodeTitle(episodeObject[ORIGINALTITLE].toString());

    QJsonArray jsonArray = episodeObject[QStringLiteral("link")].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject link = value.toObject();

        if (link[QStringLiteral("rel")].toString()==QStringLiteral("aco:web")){
            mediaTVSearchPtr->setLinkName(link[QStringLiteral("name")].toString());
            if (mediaTVSearchPtr->linkName().isEmpty()){
                mediaTVSearchPtr->setLinkName(mediaTVSearchPtr->title());
            }
            mediaTVSearchPtr->setLinkHref(link[HREF].toString());
            break;
        }
    }

    if (episodeObject[QStringLiteral("media")].isArray()){
        parseMedia(episodeObject[QStringLiteral("media")].toArray(), Scraper::SearchOption::All, mediaTVSearchPtr);
    }

    QStringList actors;
    QStringList directors;

    if (episodeObject[QStringLiteral("castMember")].isArray()){
        QJsonArray castArray = episodeObject[QStringLiteral("castMember")].toArray();
        foreach (const QJsonValue & value, castArray)
        {
            QJsonObject cast = value.toObject();

            if (cast[QStringLiteral("person")].isObject() && cast[QStringLiteral("activity")].isObject()){
                int code=cast[QStringLiteral("activity")].toObject()[CODE].toInt();
                if (code==8001){
                    actors.append(cast[QStringLiteral("person")].toObject()[QStringLiteral("name")].toString());
                } else if (code==8002){
                    directors.append(cast[QStringLiteral("person")].toObject()[QStringLiteral("name")].toString());
                }
            }
        }
    }

    mediaTVSearchPtr->setActors(mediaTVSearchPtr->actors() << actors);
    mediaTVSearchPtr->setDirectors(mediaTVSearchPtr->directors() << directors);

    if (episodeObject[QStringLiteral("broadcast")].isArray()){
        QJsonArray broadcastArray = episodeObject[QStringLiteral("broadcast")].toArray();
        foreach (const QJsonValue & value, broadcastArray)
        {
            QJsonObject broadcast = value.toObject();

            if (broadcast[QStringLiteral("channel")].isObject()){
                mediaTVSearchPtr->addNetwork( broadcast[QStringLiteral("channel")].toObject()[DOLLAR].toString());
            }

            QString dateTime=broadcast[QStringLiteral("datetime")].toString();
            QStringList d =dateTime.split('T');
            if (d.size()==2){
                QDate date = QDate::fromString(d.at(0), QStringLiteral("yyyy-MM-dd"));
                QTime time = QTime::fromString(d.at(1), QStringLiteral("h:m:s"));
                if (date.isValid() && time.isValid()){
                    mediaTVSearchPtr->setAired(QDateTime(date,time));
                }
            }
        }
    }


    return true;
}

void AlloCineScraper::findMediaInfo(QNetworkAccessManager *manager, const QString mediaCode) const{
    QMap<QString,QString> params;
    params[CODE]=QUrl::toPercentEncoding(mediaCode);
    //  params["profile"]=QUrl::toPercentEncoding("large");

    QString url=createURL(URL_MEDIA,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug() << data;
            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                //   SearchEpisodeInfo result2=result;
                //   if (parseEpisodeTVSerieInfo(doc, result2)){
                //        emit found(this,result2);
                //    }else {
                //       emit scraperError();
                //   }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

void AlloCineScraper::findEpisodeInfoByCode(QNetworkAccessManager *manager, const QString episodeCode, MediaTVSearchPtr mediaTVSearchPtr) const{

    QMap<QString,QString> params;
    params[CODE]=QUrl::toPercentEncoding(episodeCode);
    params[PROFILE]=QUrl::toPercentEncoding(LARGE);
    params[STRIPTAGS]=QUrl::toPercentEncoding(STRIPSYNOPSIS);

    QString url=createURL(URL_EPISODE,params);

    Promise* promise=Promise::loadAsync(*manager,url);

    connect(this, SIGNAL(canceled()), promise, SIGNAL(canceled()), Qt::UniqueConnection);

    QObject::connect(promise, &Promise::completed, [=]()
    {
        if (promise->replyError() ==QNetworkReply::NoError){
            const QByteArray data=promise->replyData();
            qDebug().noquote() << "findEpisodeInfoByCode:" << data;

            QJsonParseError e;
            QJsonDocument doc=  QJsonDocument::fromJson(data,&e);
            if (e.error== QJsonParseError::NoError){
                if (parseEpisodeTVSerieInfo(doc, mediaTVSearchPtr)){
                    emit found(this,mediaTVSearchPtr);
                }else {
                    emit scraperError();
                }
            } else {
                emit scraperError(e.errorString());
            }
        } else {
            emit scraperError(promise->replyErrorString());
        }
    });
}

bool AlloCineScraper::extractRating(const QJsonObject& statisticsObject, double& rating) const {

    rating=0.f;
    int diviseur=0;
    if (statisticsObject[QStringLiteral("userRating")].isDouble()){
        rating += statisticsObject[QStringLiteral("userRating")].toDouble();
        diviseur++;
    }

    if (statisticsObject[QStringLiteral("pressRating")].isDouble()){
        rating += statisticsObject[QStringLiteral("pressRating")].toDouble();
        diviseur++;
    }

    if (diviseur!=0 && rating>0.f){
        rating=double(2)*rating/double(diviseur);
        return true;
    }

    return false;

}


bool AlloCineScraper::extractCertificate(const QJsonObject& certificateObject, MediaMovieSearchPtr mediaMovieSearchPtr) const {
    auto code = certificateObject[QStringLiteral("certificate")].toObject()[CODE];
    if (!code.isNull()){
        qDebug() << code.toInt();
        switch(code.toInt()){
        case 14001:  // Interdit aux moins de 12 ans
        case 14044:  // Interdit aux moins de 12 ans avec avertissement
            mediaMovieSearchPtr->setCertificate(QStringLiteral("fr_12"));
            ; break;
        case 14002:  // Interdit aux moins de 16 ans
        case 14045:  // Interdit aux moins de 16 ans avec avertissement"
            mediaMovieSearchPtr->setCertificate(QStringLiteral("fr_16"));
            ; break;

        case 14004: // Interdit aux moins de 18 ans
            mediaMovieSearchPtr->setCertificate(QStringLiteral("fr_18"));

            ; break;
        case 14005: // Film classe X
            mediaMovieSearchPtr->setCertificate(QStringLiteral("fr_X"));

        case 14035: // Avertissement : des scÃ¨nes, des propos ou des images peuvent heurter la sensibilitÃ© des spectateurs")}
            ; break;
        }
    }

    return true;
}



void AlloCineScraper::init(){
    struct code2Country {
        int codeAlloCine;
        QLatin1String ISO3166;
    };

    static code2Country alloCineCodeToCountries[]=
    {
        {5001,QLatin1String("FR")},//France
        {5002,QLatin1String("US")},//U.S.A.
        {5003,QLatin1String("DE")},//Allemagne de l'Est
        {5004,QLatin1String("GB")},//Grande-Bretagne
        {5005,QLatin1String("NZ")},//Nouvelle-Zélande
        {5007,QLatin1String("ZA")},//Afrique du Sud
        {5008,QLatin1String("KR")},//Corée du Sud
        {5009,QLatin1String("KP")},//Corée du Nord
        {5010,QLatin1String("CH")},//Suisse
        {5012,QLatin1String("NE")},//Niger
        {5013,QLatin1String("DZ")},//Algérie
        {5014,QLatin1String("BE")},//Belgique
        {5015,QLatin1String("SO")},//Somalie
        {5016,QLatin1String("AM")},//Arménie
        {5017,QLatin1String("ES")},//Espagne
        {5018,QLatin1String("CA")},//Canada
        {5019,QLatin1String("GR")},//Grèce
        {5020,QLatin1String("IT")},//Italie
        {5021,QLatin1String("JP")},//Japon
        {5022,QLatin1String("JM")},//Jamaïque
        {5023,QLatin1String("PL")},//Pologne
        {5024,QLatin1String("PT")},//Portugal
        {5025,QLatin1String("AR")},//Argentine
        {5026,QLatin1String("TR")},//Turquie
        {5027,QLatin1String("CN")},//Chine
        {5028,QLatin1String("BR")},//Brésil
        {5029,QLatin1String("AU")},//Australie
        {5030,QLatin1String("IE")},//Irlande
        {5031,QLatin1String("MX")},//Mexique
        {5032,QLatin1String("AT")},//Autriche
        {5033,QLatin1String("MA")},//Maroc
        {5034,QLatin1String("NG")},//Nigéria
        {5036,QLatin1String("ZW")},//Zimbabwé
        {5037,QLatin1String("IL")},//Israël
        {5038,QLatin1String("ML")},//Mali
        {5039,QLatin1String("RU")},//Russie
        {5040,QLatin1String("TW")},//Taïwan
        {5041,QLatin1String("CZ")},//Tchécoslovaquie
        {5042,QLatin1String("IN")},//Inde
        {5043,QLatin1String("PS")},//Palestine
        {5044,QLatin1String("AO")},//Angola
        {5045,QLatin1String("LU")},//Luxembourg
        {5047,QLatin1String("CM")},//Cameroun
        {5048,QLatin1String("CG")},//Congo (Brazzaville)
        {5049,QLatin1String("LB")},//Liban
        {5051,QLatin1String("PE")},//Pérou
        {5052,QLatin1String("TN")},//Tunisie
        {5053,QLatin1String("CD")},//Zaïre
        {5054,QLatin1String("VN")},//Vietnam
        {5055,QLatin1String("BO")},//Bolivie
        {5056,QLatin1String("BY")},//Biélorussie
        {5057,QLatin1String("KR")},//Corée
        {5058,QLatin1String("MM")},//Birmanie
        {5059,QLatin1String("CL")},//Chili
        {5060,QLatin1String("CU")},//Cuba
        {5061,QLatin1String("DK")},//Danemark
        {5062,QLatin1String("NO")},//Norvège
        {5063,QLatin1String("EG")},//Egypte
        {5064,QLatin1String("BG")},//Bulgarie
        {5065,QLatin1String("CO")},//Colombie
        {5066,QLatin1String("XX")},//Yougoslavie
        {5067,QLatin1String("SE")},//Suède
        {5068,QLatin1String("HU")},//Hongrie
        {5069,QLatin1String("FI")},//Finlande
        {5070,QLatin1String("ID")},//Indonésie
        {5072,QLatin1String("BD")},//Bengladesh
        {5073,QLatin1String("BA")},//Bosnie-Herzégovine
        {5074,QLatin1String("BW")},//Botswana
        {5076,QLatin1String("CS")},//Serbie
        {5077,QLatin1String("HR")},//Croatie
        {5079,QLatin1String("AE")},//Emirats Arabes Unis
        {5080,QLatin1String("EE")},//Estonie
        {5081,QLatin1String("CY")},//Chypre
        {5082,QLatin1String("VE")},//Vénézuela
        {5083,QLatin1String("HT")},//Haïti
        {5084,QLatin1String("KH")},//Cambodge
        {5086,QLatin1String("IR")},//Iran
        {5087,QLatin1String("LT")},//Lituanie
        {5088,QLatin1String("RO")},//Roumanie
        {5089,QLatin1String("SN")},//Sénégal
        {5090,QLatin1String("GA")},//Gabon
        {5091,QLatin1String("TD")},//Tchad
        {5093,QLatin1String("IS")},//Islande
        {5095,QLatin1String("SY")},//Syrie
        {5097,QLatin1String("MZ")},//Mozambique
        {5098,QLatin1String("LK")},//Sri Lanka
        {5099,QLatin1String("NP")},//Népal
        {5100,QLatin1String("SK")},//Slovaquie
        {5101,QLatin1String("LV")},//Lettonie
        {5102,QLatin1String("AZ")},//Azerbaïdjan
        {5103,QLatin1String("MR")},//Mauritanie
        {5104,QLatin1String("MN")},//Mongolie
        {5105,QLatin1String("BZ")},//Belize
        {5106,QLatin1String("UA")},//Ukraine
        {5107,QLatin1String("UY")},//Uruguay
        {5108,QLatin1String("SI")},//Slovénie
        {5109,QLatin1String("TJ")},//Tadjikistan
        {5111,QLatin1String("BJ")},//Bénin
        {5112,QLatin1String("TG")},//Togo
        {5113,QLatin1String("SG")},//Singapour
        {5114,QLatin1String("SA")},//Arabie Saoudite
        {5115,QLatin1String("PH")},//Philippines
        {5116,QLatin1String("PA")},//Panama
        {5117,QLatin1String("AF")},//Afghanistan
        {5118,QLatin1String("KG")},//kirghizistan
        {5119,QLatin1String("GE")},//Géorgie
        {5120,QLatin1String("KZ")},//kazakhstan
        {5121,QLatin1String("MG")},//Madagascar
        {5122,QLatin1String("GN")},//Guinée
        {5123,QLatin1String("CI")},//Côte-d'Ivoire
        {5125,QLatin1String("GT")},//Guatemala
        {5126,QLatin1String("GH")},//Ghana
        {5127,QLatin1String("TH")},//Thaïlande
        {5128,QLatin1String("AL")},//Albanie
        {5129,QLatin1String("DE")},//Allemagne
        {5130,QLatin1String("DE")},//Allemagne de l'Ouest
        {5132,QLatin1String("CD")},//Congo (Kinshasa)
        {5133,QLatin1String("DO")},//République dominicaine
        {5134,QLatin1String("EC")},//Equateur
        {5135,QLatin1String("ER")},//Erythrée
        {5136,QLatin1String("ET")},//Ethiopie
        {5137,QLatin1String("GM")},//Gambie
        {5138,QLatin1String("GW")},//Guinée-Bissau
        {5139,QLatin1String("GQ")},//Guinée équatoriale
        {5140,QLatin1String("GY")},//Guyana
        {5141,QLatin1String("HN")},//Honduras
        {5142,QLatin1String("HK")},//Hong-Kong
        {5143,QLatin1String("JO")},//Jordanie
        {5147,QLatin1String("LY")},//Libye
        {5148,QLatin1String("MK")},//Macédoine
        {5149,QLatin1String("MY")},//Malaisie
        {5150,QLatin1String("MD")},//Moldavie
        {5152,QLatin1String("OM")},//Oman
        {5153,QLatin1String("UG")},//Ouganda
        {5154,QLatin1String("UZ")},//Ouzbékistan
        {5155,QLatin1String("PK")},//Pakistan
        {5156,QLatin1String("PY")},//Paraguay
        {5158,QLatin1String("RW")},//Rwanda
        {5159,QLatin1String("SV")},//Salvador
        {5161,QLatin1String("XX")},//U.R.S.S.
        {5164,QLatin1String("TZ")},//Tanzanie
        {5165,QLatin1String("CZ")},//République tchèque
        {5166,QLatin1String("TM")},//Turkménistan
        {5167,QLatin1String("ZM")},//Zambie
        {5171,QLatin1String("MC")},//Monaco
        {5173,QLatin1String("BF")},//Burkina Faso
        {5174,QLatin1String("IQ")},//Irak
        {5177,QLatin1String("NL")},//Pays-Bas
        {7236,QLatin1String("XX")},//Porto Rico
        {7239,QLatin1String("XX")},//Québec
        {7240,QLatin1String("XX")},//Indéfini
        {7241,QLatin1String("KE")},//Kenya
        {7244,QLatin1String("BB")},//Barbade
        {7246,QLatin1String("BS")},//Bahamas
        {7247,QLatin1String("CV")},//Cap-Vert
        {7249,QLatin1String("DM")},//Dominique
        {7254,QLatin1String("LC")},//Sainte-Lucie
        {7255,QLatin1String("ME")},//Monténégro
        {7257,QLatin1String("MU")},//Maurice
        {7264,QLatin1String("TT")},//Trinité-et-Tobago
        {7268,QLatin1String("WS")},//Samoa
        {7270,QLatin1String("CR")} //Costa Rica
    };

    if (codesToCountries.isEmpty()){
        for (code2Country c : alloCineCodeToCountries){
            codesToCountries[c.codeAlloCine]=QString(c.ISO3166).toLower();
        }
    }
}

bool AlloCineScraper::extractNationality(const QJsonArray& nationalityArray, MediaMovieSearchPtr mediaMovieSearchPtr) const {
    QStringList countries;

    for (const QJsonValue & value : nationalityArray){
        if (JSonHelper::isInt(value.toObject()[CODE])){
            int code=value.toObject()[CODE].toInt();
            if (codesToCountries.contains(code)){
                countries << codesToCountries[code];
            }
        }

    }
    mediaMovieSearchPtr->setCountries(countries);

    return true;
}

bool AlloCineScraper::parseMovieInfo(QNetworkAccessManager *manager, const QJsonDocument& resultset, const SearchFor& searchFor, MediaMovieSearchPtr mediaMovieSearchPtr) const{

    Q_UNUSED(manager);

    if (!resultset.isObject()){
        return false;
    }
    QJsonObject jsonObject = resultset.object();

    if (!jsonObject[MOVIE].isObject()){
        return false;
    }

    QJsonObject movieObject = jsonObject[MOVIE].toObject();

    mediaMovieSearchPtr->setTitle(movieObject[TITLE].toString());
    mediaMovieSearchPtr->setOriginalTitle(movieObject[ORIGINALTITLE].toString());
    mediaMovieSearchPtr->setSynopsis(movieObject[QStringLiteral("synopsis")].toString());
    mediaMovieSearchPtr->setProductionYear(movieObject[QStringLiteral("productionYear")].toInt());
    mediaMovieSearchPtr->setRuntimeInSec(movieObject[RUNTIME].toInt());

    extractNationality(movieObject[QStringLiteral("nationality")].toArray(),mediaMovieSearchPtr);
    extractCertificate(movieObject[QStringLiteral("movieCertificate")].toObject(),mediaMovieSearchPtr);

    extractGenre<MediaMovieSearchPtr>(movieObject,mediaMovieSearchPtr);


    //    if(movieObject["poster"].isObject()){
    //        info.posterHref = movieObject["poster"].toObject()["href"].toString();
    //        info.postersHref.append(info.posterHref);
    //    }

    if(movieObject[QStringLiteral("castingShort")].isObject()){
        QStringList directors = movieObject[QStringLiteral("castingShort")].toObject()[QStringLiteral("directors")].toString().split(',', QString::SkipEmptyParts);
        directors.replaceInStrings(QRegExp("^\\s+"),"");
        mediaMovieSearchPtr->setDirectors(mediaMovieSearchPtr->directors() << directors);

        QStringList actors=movieObject[QStringLiteral("castingShort")].toObject()[QStringLiteral("actors")].toString().split(',', QString::SkipEmptyParts);
        actors.replaceInStrings(QRegExp("^\\s+"),"");
        mediaMovieSearchPtr->setActors(mediaMovieSearchPtr->actors() << actors);
    }


    if (movieObject[QStringLiteral("media")].isArray()){
        if (searchFor & SearchOption::AllMedia){
            parseMedia(movieObject[QStringLiteral("media")].toArray(), searchFor, mediaMovieSearchPtr);
        }
    }


    if (movieObject[QStringLiteral("statistics")].isObject()){
        double rating;
        if (extractRating(movieObject[QStringLiteral("statistics")].toObject(),rating)){
            mediaMovieSearchPtr->setRating(rating);
        }
    }

    return true;
}


QString AlloCineScraper::getBestImageUrl(const QString& filePath, const QSize& originalSize,const QSize& size, Qt::AspectRatioMode mode, QFlags<ImageType> imageType) const{
    Q_UNUSED(mode);
    Q_UNUSED(imageType);

    qDebug() << filePath;

    if (!originalSize.isNull() && size.expandedTo(originalSize)==size){
        return filePath;
    } else {

        QUrl url(filePath);
        QSize scaledSize = originalSize.scaled(size, mode);
        // Cf. https://raw.githubusercontent.com/etienne-gauvin/api-allocine-helper/master/AlloImage.class.php
        QString result= QString (QStringLiteral("http://%1/r_%2_%3%4")).arg(url.host()).arg(scaledSize.width()).arg(scaledSize.height()).arg(url.path());
        qDebug() << result;
          return result;
    }
}

FilmPrtList AlloCineScraper::parseResultset(const QJsonDocument& resultset) const{

    FilmPrtList films;

    if (!resultset.isObject()){
        return films;
    }

    QJsonObject jsonObject = resultset.object();

    if (!jsonObject[QStringLiteral("feed")].isObject()){
        return films;
    }

    QJsonObject feedObject = jsonObject[QStringLiteral("feed")].toObject();

    if (!feedObject[MOVIE].isArray()){
        return films;
    }

    QJsonArray jsonArray = feedObject[MOVIE].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        FilmPtr film(new Film());

        film->originalTitle= obj[ORIGINALTITLE].toString();
        film->title= obj[TITLE].toString();
        if (film->title.isEmpty()){
            film->title=film->originalTitle;
        }
        film->productionYear = QString::number(obj[QStringLiteral("productionYear")].toDouble());
        film->code= QString::number(obj["code"].toDouble());
        //        if(obj["poster"].isObject()){
        //            film->posterHref = obj["poster"].toObject()["href"].toString();
        //        }



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

    if (!jsonObject[QStringLiteral("feed")].isObject()){
        return shows;
    }

    QJsonObject feedObject = jsonObject[QStringLiteral("feed")].toObject();

    if (!feedObject[TVSERIES].isArray()){
        return shows;
    }

    QJsonArray jsonArray = feedObject[TVSERIES].toArray();

    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        ShowPtr show(new Show());
        show->originalTitle= obj[ORIGINALTITLE].toString();
        show->title= obj[ORIGINALTITLE].toString();
        show->productionYear = QString::number(obj[QStringLiteral("yearStart")].toDouble());
        show->code= QString::number(obj[CODE].toDouble());
        if(obj[QStringLiteral("poster")].isObject()){
            show->posterHref = obj[QStringLiteral("poster")].toObject()[HREF].toString();
        }

        shows.append(show);
    }

    return shows;
}

QIcon AlloCineScraper::loadIcon() const {

    static const uchar icon_png[] = {
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


    QPixmap pixmap;
    if (pixmap.loadFromData(icon_png, sizeof(icon_png)/sizeof(uchar))){
        return QIcon(pixmap);
    }

    return QIcon();
}

QString AlloCineScraper::name() const{
    return QStringLiteral("Allocine");
}

QIcon AlloCineScraper::icon() const {
    return m_icon;
}
bool AlloCineScraper::haveCapability(const SearchCapabilities capability) const {
    Q_UNUSED(capability);
    return true;
}

bool AlloCineScraper::supportLanguage(const QString& languageCodeISO639) const {
    return QStringLiteral("fr")==languageCodeISO639;
}
