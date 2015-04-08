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

    void findMovieInfoGetImage(QNetworkAccessManager* manager, const QString& movieCode, const SearchFor &searchFor,  SearchMovieInfo &result) const;
    void findEpisodeInfoGetImage(QNetworkAccessManager* manager, const QString& showCode, const int season, const int episode,  SearchEpisodeInfo& result) const;
    void findEpisodeInfoGetCredit(QNetworkAccessManager* manager, const QString& showCode, const int season, const int episode,  SearchEpisodeInfo& result) const;


    bool parseImageInfo(const QJsonDocument& resultset, const SearchFor &searchFor, SearchMovieInfo& info) const;
    bool parseImageInfo(const QJsonDocument& resultset, const SearchFor &searchFor, SearchEpisodeInfo& info) const;
    bool parseCreditInfo(const QJsonDocument& resultset, SearchEpisodeInfo& info) const;

    QStringList posterSizes;
    QStringList backdropSizes;

    void searchFilmConfigurationOk(QNetworkAccessManager* manager, const QString& toSearch, int year, const QString &language);
    void searchTVConfigurationOk(QNetworkAccessManager* manager, const QString& toSearch, const QString &language);

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode, const SearchFor &searchFor, const QString& language) const;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const QString& language) const ;

public:
    TheMovieDBScraper(QObject *parent=0);
    QIcon getIcon() const;
    QString getName() const;
    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio, ImageType imageType=ImageType::UNKNOWN) const;
};

#endif // THEMOVIEDBSCRAPER_H
