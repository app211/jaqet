#ifndef THEMOVIEDBSCRAPER_H
#define THEMOVIEDBSCRAPER_H

#include "scraper.h"

class TheMovieDBScraper: public Scraper
{
    Q_OBJECT

    QIcon m_icon;

    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument&) const;
    bool parseConfiguration(const QJsonDocument& );
    QString  baseUrl;
    bool parseMovieInfo(const QJsonDocument& resultset,  MediaMovieSearchPtr mediaMovieSearchPtr) const;
    bool parseEpisodeInfo(const QJsonDocument& resultset, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode) const;

    void findMovieInfoGetImage(QNetworkAccessManager* manager, const QString& movieCode, const SearchFor &searchFor,  MediaMovieSearchPtr mediaMovieSearchPtr) const;
    void findEpisodeInfoGetImage(QNetworkAccessManager* manager, const QString& showCode, const int season, const int episode,  MediaTVSearchPtr mediaTVSearchPtr) const;
    void findEpisodeInfoGetCredit(QNetworkAccessManager* manager, const QString& showCode, const int season, const int episode,  MediaTVSearchPtr mediaTVSearchPtr) const;


    bool parseImageInfo(const QJsonDocument& resultset, const SearchFor &searchFor, MediaMovieSearchPtr mediaMovieSearchPtr) const;
    bool parseImageInfo(const QJsonDocument& resultset, const SearchFor &searchFor, MediaTVSearchPtr mediaTVSearchPtr) const;
    bool parseCreditInfo(const QJsonDocument& resultset, MediaTVSearchPtr mediaTVSearchPtr) const;

    QStringList posterSizes;
    QStringList backdropSizes;

    void searchFilmConfigurationOk(QNetworkAccessManager* manager, const QString& toSearch, int year, const QString &language);
    void searchTVConfigurationOk(QNetworkAccessManager* manager, const QString& toSearch, const QString &language);
    QIcon loadIcon() const;

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) ;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) ;
    void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor &searchFor, const QString& language) ;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor &searchFor, const QString& language)  ;

public:
    TheMovieDBScraper(QObject *parent=0);
    QIcon icon() const override final;
    QString name() const;

    bool haveCapability(const SearchCapabilities capability) const override final;
    bool supportLanguage(const QString& languageCodeISO639) const override final;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All) const;
};

#endif // THEMOVIEDBSCRAPER_H
