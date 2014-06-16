#ifndef THEMOVIEDBSCRAPER_H
#define THEMOVIEDBSCRAPER_H

#include "scraper.h"

class TheMovieDBScraper: public Scraper
{
    Q_OBJECT

    static const QString API_KEY;

    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument&) const;
    bool parseConfiguration(const QJsonDocument& );
    QString  baseUrl;
    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    bool parseEpisodeInfo(const QJsonDocument& resultset, SearchEpisodeInfo& info, const int season, const int episode) const;

    void findMovieInfoGetImage(QNetworkAccessManager* manager, const QString& movieCode,  SearchMovieInfo &result) const;
    bool parseImageInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;
    QStringList posterSizes;
    QStringList backdropSizes;
    bool findSaisonInfo(const QString& showCode, const QString& season, SearchEpisodeInfo &result) const;
    bool findTVInfo(const QString& showCode, SearchEpisodeInfo &result) const;
    void searchFilmConfigurationOk( QNetworkAccessManager* manager, const QString& toSearch);
    void searchTVConfigurationOk(QNetworkAccessManager* manager, const QString& toSearch);

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language);

public:
    TheMovieDBScraper();
    QIcon getIcon() const;
   QString getName() const;
   QString createURL(const QString& , const QMap<QString, QString>& params) const;
    void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const;
    void findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const ;
    QString getBestImageUrl(const QString& filePath, const QSize& size) const;



};

#endif // THEMOVIEDBSCRAPER_H
