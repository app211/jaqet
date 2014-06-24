#ifndef ALLOCINESCRAPER_H
#define ALLOCINESCRAPER_H

#include "scraper.h"

class AlloCineScraper : public Scraper
{
    static const QString ALLO_DEFAULT_URL_IMAGES;
    static const uchar icon_png[];
    FilmPrtList parseResultset(const QJsonDocument& ) const;
    ShowPtrList parseTVResultset(const QJsonDocument& resultset) const;

    bool parseMovieInfo(const QJsonDocument& resultset, SearchMovieInfo& info) const;

    bool extractSeasonCodeFromLargeTVSerieInfo(const QJsonDocument& resultset, const int seasonToFind, QString &seasonCode, SearchEpisodeInfo& result) const;
    bool extractEpisodeCodeFromLargeSeasonTVSerieInfo(const QJsonDocument& resultset, const int episodeToFind, QString& episodeCode, SearchEpisodeInfo& result) const;
    void findSeasonInfoByCode(QNetworkAccessManager *manager, const QString seasonCode, const int episode, SearchEpisodeInfo result) const;
    void findEpisodeInfoByCode(QNetworkAccessManager *manager, const QString epidodeCode, SearchEpisodeInfo result) const;

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
