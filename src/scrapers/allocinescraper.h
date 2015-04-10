#ifndef ALLOCINESCRAPER_H
#define ALLOCINESCRAPER_H

#include "scraper.h"

class AlloCineScraper : public Scraper
{
    Q_OBJECT

    static const QString ALLO_DEFAULT_URL_IMAGES;
    static const uchar icon_png[];
    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument& resultset) const;

    bool parseMovieInfo(QNetworkAccessManager *manager, const QJsonDocument& resultset, const SearchFor &searchFor, SearchMovieInfo &info) const;

    bool extractSeasonCodeFromLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString &seasonCode, SearchEpisodeInfo& result) const;
    bool extractEpisodeCodeFromLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode, SearchEpisodeInfo& result) const;
    void findSeasonInfoByCode(QNetworkAccessManager *manager, const QString seasonCode, const int episode, SearchEpisodeInfo result) const;
    void findEpisodeInfoByCode(QNetworkAccessManager *manager, const QString epidodeCode, SearchEpisodeInfo result) const;
    void findMediaInfo(QNetworkAccessManager *manager, const QString mediaCode) const;

protected :
    void internalSearchFilm( QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode, const SearchFor &searchFor, const QString& language) ;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const SearchFor &searchFor,  const QString& language) ;

public:
    AlloCineScraper(QObject *parent=0);

    QIcon getIcon() const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio, ImageType imageType=ImageType::UNKNOWN ) const;

};
#endif // ALLOCINESCRAPER_H
