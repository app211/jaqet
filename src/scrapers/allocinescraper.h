#ifndef ALLOCINESCRAPER_H
#define ALLOCINESCRAPER_H

#include "scraper.h"

class AlloCineScraper : public Scraper
{
    Q_OBJECT

    static const QString ALLO_DEFAULT_URL_IMAGES;
    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument& resultset) const;

    bool parseMovieInfo(QNetworkAccessManager *manager, const QJsonDocument& resultset, const SearchFor &searchFor, MediaMovieSearchPtr mediaMovieSearchPtr) const;
    bool parseEpisodeTVSerieInfo(const QJsonDocument& resultset, MediaTVSearchPtr mediaTVSearchPtr  ) const;
    bool parseMedia(const QJsonArray& mediaArray,const Scraper::SearchFor& searchFor,  MediaTVSearchPtr mediaTVSearchPtr ) const ;
    bool parseMedia(const QJsonArray& mediaArray,const Scraper::SearchFor& searchFor,  MediaMovieSearchPtr mediaMovieSearchPtr ) const ;

    bool extractSeasonCodeFromLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString &seasonCode, MediaTVSearchPtr mediaTVSearchPtr) const;
    bool extractEpisodeCodeFromLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode, MediaTVSearchPtr mediaTVSearchPtr) const;
    void findSeasonInfoByCode(QNetworkAccessManager *manager, const QString seasonCode, const int episode, MediaTVSearchPtr mediaTVSearchPtr) const;
    void findEpisodeInfoByCode(QNetworkAccessManager *manager, const QString epidodeCode, MediaTVSearchPtr mediaTVSearchPtr) const;
    void findMediaInfo(QNetworkAccessManager *manager, const QString mediaCode) const;

    bool getRatingFromStatistics(const QJsonObject& statisticsObject, double& rating) const;

    QIcon _icon;
    void initIcon();

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor &searchFor, const QString& language) ;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor &searchFor,  const QString& language)  ;

public:
    AlloCineScraper(QObject *parent=0);

    QIcon getIcon() const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All ) const;

};
#endif // ALLOCINESCRAPER_H
