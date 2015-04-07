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
    QString originalTitle;
    QString title;
    QString synopsis;
    QStringList backdropsHref;
    QList<QSize> backdropsSize;
    QStringList postersHref;
    QList<QSize> postersSize;
    QString linkName;
    QString linkHref;
    QStringList directors;
    QStringList actors;
    int productionYear;
    int runtime;
    double rating;
};

class SearchEpisodeInfo {
public:
    QString code;
    QString title;
    QString originalTitle;
    QString episodeTitle;
    QString originalEpisodeTitle;
    QString synopsis;
    QStringList backdropsHref;
    QList<QSize> backdropsSize;
    QList<QSize> postersSize;
    QStringList postersHref;
    QStringList bannersHref;
    QString linkName;
    QString linkHref;
    int season;
    int episode;
    QString showTitle;
    QString originalShowTitle;
    int productionYear=0;
    QStringList actors;
    int runtime;
    double rating;
    QStringList directors;
    QString network;

};

class Scraper;

struct FoundResult {

    FoundResult() {
    }

    FoundResult(Scraper *scraper, const FilmPtr& filmPtr)
        :
          scraper(scraper),
          filmPtr(filmPtr)
    {
    }

    FoundResult(Scraper *scraper, const ShowPtr& showPtr, const int season, const int episode)
        : scraper(scraper),
          showPtr( showPtr)
        , season(season)
        ,episode(episode)
    {
    }

    bool isNull() const {
        return showPtr.isNull() && filmPtr.isNull();
    }

    bool isTV() const {
        return !showPtr.isNull();
    }

    QString getCode() const {
        return isNull()?"":(isTV()?showPtr->code:filmPtr->code);
    }

    QString getOriginalTitle() const {
        return isNull()?"":(isTV()?showPtr->originalTitle:filmPtr->originalTitle);

    }
    Scraper *getScraper() const {
        return scraper;
    }

    int getSeason() const{
        return isTV()?season:-1;
    }

    int getEpisode() const{
        return  isTV()?episode:-1;
    }

    QString getProductionYear() const{
        return isNull()?"":(isTV()?showPtr->productionYear:filmPtr->productionYear);

    }

    Scraper *scraper;
    int season;
    int episode;
    ShowPtr showPtr;
    FilmPtr filmPtr;
};


class Scraper : public QObject
{
    Q_OBJECT

public:

    enum SearchCapabilities {
        Movie = 0x0001,
        TV = 0x0002
    };

    Scraper(QObject *parent=0);

    virtual bool haveCapability(const SearchCapabilities capability) const{
        return true;
    }

    virtual QIcon getIcon() const =0;
    virtual QString getName() const = 0;

    virtual QString createURL(const QString& , const QMap<QString, QString>& params) const=0;

    void searchFilm(QNetworkAccessManager* manager, const QString& toSearch, int year) const;
    void searchTV(QNetworkAccessManager* manager, const QString& toSearch) ;
    void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const;
    void findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const;

    enum class ImageType {
        UNKNOWN, BANNER, POSTER,BACKDROP
    };

    virtual QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio,ImageType imageType=ImageType::UNKNOWN) const=0;

protected:
    virtual void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const=0;
    virtual void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const=0;
    virtual void internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode, const QString& language) const=0;
    virtual void internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const QString& language) const=0;

public slots:
    void closeDialog();
    void showErrorDialog(const QString& error=QString::null);

Q_SIGNALS:
    void scraperError() const;
    void scraperError(const QString& error) const;
    void found(FilmPrtList films) const;
    void found(ShowPtrList shows) const;
    void found(const Scraper* scraper,SearchMovieInfo films) const;
    void found(const Scraper* scraper,SearchEpisodeInfo films) const;
    void progress(const QString& progressInfo) const;
 };


class ScraperResource {
public:
    QString resources;
    QSize originalSize;
    const Scraper* scraper;
    ScraperResource(const QString& resources, const QSize& originalSize, const Scraper* scraper){
        this->resources=resources;
        this->scraper=scraper;
        this->originalSize=originalSize;
    }

    ScraperResource(){

    }
};

Q_DECLARE_METATYPE(ScraperResource);

#endif // SCRAPER_H
