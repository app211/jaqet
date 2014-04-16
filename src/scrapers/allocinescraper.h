#ifndef ALLOCINESCRAPER_H
#define ALLOCINESCRAPER_H

#include "scraper.h"

class AlloCineScraper : public Scraper
{
    static const QString ALLOCINE_SECRET_KEY;
    static const QString ALLO_DEFAULT_URL_API;
    static const QString ALLO_DEFAULT_URL_IMAGES;
    static const QString ALLO_PARTNER;

    QList<Film> parseResultset(const QJsonDocument& ) const;
    QList<Show> parseTVResultset(const QJsonDocument& resultset) const;
    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    bool parseLargeTVSerieInfo(const QJsonDocument& resultset, const QString &season, QString &seasonCode) const;
 bool parseLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const QString&  episode, QString& episodeCode) const;
 bool findSaisonInfo(const QString& seasonCode, const QString& episode, QString &episodeCode) const;
 bool findEpisodeInfo(const QString& epidodeCode, SearchEpisodeInfo &result) const;

public:
    AlloCineScraper();
     bool findMovieInfo(const QString& movieCode, SearchMovieInfo &result) const;
     bool findEpisodeInfo(const QString& showCode, const QString&  season, const QString& epidode, SearchEpisodeInfo &result) const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    bool searchFilm(const QString& toSearch, SearchResult &result) ;
    bool searchTV(const QString& toSearch, SearchTVResult &result);

    QString getBestImageUrl(const QString& filePath, const QSize& size) const;

};
#endif // ALLOCINESCRAPER_H
