#ifndef THEMOVIEDBSCRAPER_H
#define THEMOVIEDBSCRAPER_H

#include "scraper.h"

class TheMovieDBScraper: public Scraper
{
    static const QString API_KEY;
    static const uchar icon_png[];
    QIcon m_icon;

    FilmPrtList parseResultset(const QJsonDocument& ) const;
    QList<Show> parseTVResultset(const QJsonDocument&) const;
    bool parseConfiguration(const QJsonDocument& );
    QString  baseUrl;
    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;


    bool getImage(const QString& movieCode, SearchMovieInfo& info) const;
    bool parseImageInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    QStringList posterSizes;
    QStringList backdropSizes;
    bool findSaisonInfo(const QString& showCode, const QString& season, SearchEpisodeInfo &result) const;
    bool findTVInfo(const QString& showCode, SearchEpisodeInfo &result) const;

public:
    TheMovieDBScraper();
    QIcon getIcon();
    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    bool searchFilm(const QString& toSearch, SearchResult &result)  ;
    bool searchTV(const QString& toSearch, SearchTVResult &result);
    bool findMovieInfo(const QString& movieCode, SearchMovieInfo &result) const;
    bool findEpisodeInfo(const QString& showCode, const QString&  season, const QString& episode, SearchEpisodeInfo &result) const ;
    QString getBestImageUrl(const QString& filePath, const QSize& size) const;
};

#endif // THEMOVIEDBSCRAPER_H
