#ifndef DEFAULTSCRAPER_H
#define DEFAULTSCRAPER_H

#include "scraper.h"
#include <QList>
#include <QPair>

class DefaultScraper : public Scraper
{
    Q_OBJECT

    static const uchar icon_png[];

    Scraper* m_bannerScraper;
    Scraper* m_bannerScraper2;

    QString showCode;
    int season,  episode;
    SearchFor searchFor;
    QString language;
    QNetworkAccessManager *manager;
    //SearchEpisodeInfo searchEpisodeInfo;
    Scraper* currentScraper;
    SearchFor currentSearchFor;

    QList<QPair<Scraper*,SearchFor>> scrapers;
    QList<QPair<Scraper*,SearchFor>> scrapers2;

    void doFindEpisodeInfo();

protected:
    void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode, const SearchFor& searchFor, const QString& language) ;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const SearchFor& searchFor, const QString& language) ;

    QString postMirrorCreateURL(const QString& , const QMap<QString, QString>& params) const;

public:
    DefaultScraper(QObject *parent=0);
    QIcon getIcon()const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& url,const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All) const;

    bool haveCapability(const SearchCapabilities capability) const;

private slots:
    void _searchScraper();
    void _found(FilmPrtList result);
    void _found(ShowPtrList shows);
    void _found(const Scraper* scraper,MediaMovieSearchPtr films) ;
    void _found(const Scraper* scraper, MediaTVSearchPtr shows) ;

};

#endif // DEFAULTSCRAPER_H
