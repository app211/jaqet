#ifndef ALLOCINESCRAPER_H
#define ALLOCINESCRAPER_H

#include "scraper.h"

class AlloCineScraper : public Scraper
{
    static const QString ALLOCINE_SECRET_KEY;
    static const QString ALLO_DEFAULT_URL_API;
    static const QString ALLO_DEFAULT_URL_IMAGES;
    static const QString ALLO_PARTNER;
    static const uchar icon_png[];
    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument& resultset) const;
    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    bool parseLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString &seasonCode) const;
    bool parseLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode) const;
    void findSaisonInfo(QNetworkAccessManager *manager, const QString& seasonCode, const int episode) const;

    bool findEpisodeInfo(QNetworkAccessManager *manager, const QString& epidodeCode) const;

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) ;

public:
    AlloCineScraper();

    QIcon getIcon() const;

    QString getName() const;

    void  findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const;

     void findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;


    QString getBestImageUrl(const QString& filePath, const QSize& ) const;

};
#endif // ALLOCINESCRAPER_H
