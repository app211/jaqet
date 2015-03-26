#ifndef THETVDBSCRAPER_H
#define THETVDBSCRAPER_H

#include "scraper.h"

class TheTVDBScraper : public Scraper
{
    static const QString API_KEY;
    static const uchar icon_png[];

private:
    void parseSeriesList( QNetworkAccessManager* manager, const QString& toSearch, const QByteArray& data, const QString& language);
    bool parseMirrorList( const QByteArray& data);
    QString getXMLURL() const ;
    QString getZIPURL() const ;
    QString getBannerURL() const ;

protected:
    void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, const QString& movieCode, const QString& language) const;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const QString& language) const;

    QString postMirrorCreateURL(const QString& , const QMap<QString, QString>& params) const;

public:
    TheTVDBScraper(QObject *parent=0);
    QIcon getIcon()const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& url,const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode=Qt::KeepAspectRatio, ImageType imageType=ImageType::UNKNOWN) const;

    bool haveCapability(const SearchCapabilities capability) const{
        return capability==TV;
    }
};

#endif // THETVDBSCRAPER_H
