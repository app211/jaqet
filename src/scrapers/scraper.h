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
typedef QSharedPointer<Show> ShowPtr;
typedef QList<ShowPtr> ShowPtrList;

class SearchMovieInfo {
public:
    bool searchOk;
    QString error;
    QString originalTitle;
    QString title;
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
    int season;
    int episode;
    QString showTitle;
    QString originalShowTitle;

};

class Scraper : public QObject
{
    Q_OBJECT
protected:
    static int randInt(int low, int high) ;
    static QString getRandomUserAgent() ;

public:
    Scraper();
    virtual QIcon getIcon() const { return QIcon();}
    virtual QString createURL(const QString& , const QMap<QString, QString>& params) const=0;

    virtual void searchFilm(QNetworkAccessManager* manager, const QString& toSearch) =0;
    virtual void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const=0;

    virtual void searchTV(QNetworkAccessManager* manager, const QString& toSearch) =0;
    virtual void findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const=0;

    virtual QString getBestImageUrl(const QString& filePath, const QSize& size) const=0;

Q_SIGNALS:
    void found(FilmPrtList films);
    void found(ShowPtrList shows);
    void found(const Scraper* scraper,SearchMovieInfo films);
    void found(const Scraper* scraper,SearchEpisodeInfo films);
 };


class ScraperResource {
public:
    QString resources;
    const Scraper* scraper;
    ScraperResource(const QString& resources, const Scraper* scraper){
        this->resources=resources;
        this->scraper=scraper;
    }

    ScraperResource(){

    }
};

#endif // SCRAPER_H
