#ifndef THETVDBSCRAPER_H
#define THETVDBSCRAPER_H

#include "scraper.h"
#include <QXmlStreamReader>

class TheTVDBScraper : public Scraper
{
       static const QString API_KEY;
    static const uchar icon_png[];

private:
    void parseSeriesList( QNetworkAccessManager* manager, const QString& toSearch, const QByteArray& data, const QString& language);
    bool parseMirrorList( const QByteArray& data);
    bool parseEpisode(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode, bool& foundEpisode);
    bool parseSeries(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode );
    bool parseInfo(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode);
    bool parseBanner(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr);
    bool parseActors(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr);

    QString getXMLURL() const ;
    QString getZIPURL() const ;
    QString getBannerURL() const ;

    QMap<QString,QString> genreTranslator;

protected:
    void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
    void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) ;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language) ;

    QString postMirrorCreateURL(const QString& , const QMap<QString, QString>& params) const;

public:
    TheTVDBScraper(QObject *parent=0);
    QIcon getIcon()const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& url, const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All) const;

    bool haveCapability(const SearchCapabilities capability) const{
        return capability==TV;
    }
};

#endif // THETVDBSCRAPER_H
