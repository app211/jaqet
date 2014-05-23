#ifndef SCRAPER_H
#define SCRAPER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QList>
#include <QStringList>
#include <QSize>
#include <QSharedPointer>
#include <QIcon>

class QNetworkAccessManager;

class Film {
public:
    QString code;
    QString originalTitle;
    QString title;
    QString productionYear;
    QString posterHref;

};

class Show {
public:
    QString code;
    QString originalTitle;
    QString title;
    QString productionYear;
    QString posterHref;
};

typedef QSharedPointer<Film> FilmPtr;
typedef QList<FilmPtr> FilmPrtList;

class SearchResult {
public:
    bool searchOk;
    QString error;
    FilmPrtList films;
};


class SearchTVResult {
public:
    bool searchOk;
    QString error;
    QList<Show> shows;
};
class SearchMovieInfo {
public:
    bool searchOk;
    QString error;
    QString synopsis;
    QString posterHref;
    QString backdropHref;
    QStringList backdropsHref;
    QStringList postersHref;
    QString linkName;
    QString linkHref;
    QStringList directors;
    QStringList actors;
    int productionYear;
    int runtime;

};
class SearchEpisodeInfo {
public:
    bool searchOk;
    QString code;
    QString error;
    QString title;
    QString originalTitle;
    QString synopsis;
    QString posterHref;
    QString backdropHref;
    QStringList backdropsHref;
    QStringList postersHref;
    QString linkName;
    QString linkHref;
};

class Scraper : public QObject
{
    Q_OBJECT
protected:
    static int randInt(int low, int high) ;
    static QString getRandomUserAgent() ;
    static size_t curl_write_data(void *data, size_t size, size_t nmemb, void *pointer);
    static size_t curl_header(void *data, size_t size, size_t nmemb, void *pointer);
    bool execCommand(const QString& url, QMap<QString,QString>& params, QByteArray& headerData,QByteArray& data) const;

public:
    Scraper();
    virtual QIcon getIcon() { return QIcon();}
    virtual QString createURL(const QString& , const QMap<QString, QString>& params) const=0;

    virtual void searchFilm(QNetworkAccessManager* manager, const QString& toSearch) =0;
    virtual void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const=0;

    virtual bool searchTV(const QString& toSearch, SearchTVResult &result) =0;

    virtual bool findEpisodeInfo(const QString& showCode, const QString&  season, const QString& epidode, SearchEpisodeInfo &result) const=0;
    virtual QString getBestImageUrl(const QString& filePath, const QSize& size) const=0;

Q_SIGNALS:
    void found(FilmPrtList films);
    void found(SearchMovieInfo films);
};


class ScraperResource {
public:
    QString resources;
    Scraper* scraper;
    ScraperResource(const QString& resources, Scraper* scraper){
        this->resources=resources;
        this->scraper=scraper;
    }

    ScraperResource(){

    }
};

#endif // SCRAPER_H
