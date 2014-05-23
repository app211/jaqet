#ifndef THEMOVIEDBSCRAPER_H
#define THEMOVIEDBSCRAPER_H

#include "scraper.h"

class TheMovieDBScraper: public Scraper
{
    Q_OBJECT

    static const QString API_KEY;
    static const uchar icon_png[];
    QIcon m_icon;

    FilmPrtList parseResultset(const QJsonDocument& ) const;
    QList<Show> parseTVResultset(const QJsonDocument&) const;
    bool parseConfiguration(const QJsonDocument& );
    QString  baseUrl;
    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;

    void findMovieInfoGetImage(QNetworkAccessManager* manager, const QString& movieCode,  SearchMovieInfo &result) const;
    bool parseImageInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    QStringList posterSizes;
    QStringList backdropSizes;
    bool findSaisonInfo(const QString& showCode, const QString& season, SearchEpisodeInfo &result) const;
    bool findTVInfo(const QString& showCode, SearchEpisodeInfo &result) const;
    void searchFilmConfigurationOk( QNetworkAccessManager* manager, const QString& toSearch);

public:
    TheMovieDBScraper();
    QIcon getIcon();
    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    void searchFilm( QNetworkAccessManager* manager, const QString& toSearch)  ;
    bool searchTV(const QString& toSearch, SearchTVResult &result);
    void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const;
    bool findEpisodeInfo(const QString& showCode, const QString&  season, const QString& episode, SearchEpisodeInfo &result) const ;
    QString getBestImageUrl(const QString& filePath, const QSize& size) const;


};

#endif // THEMOVIEDBSCRAPER_H
