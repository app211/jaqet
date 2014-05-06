#ifndef THETVDBSCRAPER_H
#define THETVDBSCRAPER_H

#include "scraper.h"

class TheTVDBScraper : public Scraper
{
    static const QString API_KEY;
    static const uchar icon_png[];
    QIcon m_icon;

    QString createMirrorUrl() const;
    void updateMirrors();

public:
    TheTVDBScraper();
    QIcon getIcon();

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    bool searchFilm(const QString& toSearch, SearchResult &result)  ;
    bool searchTV(const QString& toSearch, SearchTVResult &result);
    bool findMovieInfo(const QString& movieCode, SearchMovieInfo &result) const;
    bool findEpisodeInfo(const QString& showCode, const QString&  season, const QString& episode, SearchEpisodeInfo &result) const ;
    QString getBestImageUrl(const QString& filePath, const QSize& size) const;
};

#endif // THETVDBSCRAPER_H
