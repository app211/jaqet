#ifndef THETVDBSCRAPER_H
#define THETVDBSCRAPER_H

#include "scraper.h"
#include <QXmlStreamReader>

class TheTVDBScraper : public Scraper
{
    QIcon m_icon;

private:
    void parseSeriesList( QNetworkAccessManager* manager, const QString& toSearch, const QByteArray& data, const QString& language);
    bool parseMirrorList( const QByteArray& data);
    bool parseEpisode(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode, bool& foundEpisode);
    bool parseSeries(QXmlStreamReader& xml, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode );
    bool parseInfo(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr, const int season, const int episode);
    bool parseBanner(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr);
    bool parseActors(const QByteArray &data, MediaTVSearchPtr mediaTVSearchPtr);
    bool parseLanguageList( const QByteArray& data);

    void retriveLanguages(QNetworkAccessManager* manager) ;

    QString getXMLURL() const ;
    QString getZIPURL() const ;
    QString getBannerURL() const ;

    QMap<QString,QString> genreTranslator;
    QIcon loadIcon() const;

protected:
    void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year)  override ;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language)  override;
    void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) override;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language) override ;

   // QString postMirrorCreateURL(const QString& , const QMap<QString, QString>& params) const;

public:
    TheTVDBScraper(QObject *parent=0);
    QIcon icon() const override final;
    QString name() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& url, const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All) const;

    bool haveCapability(const SearchCapabilities capability) const override final;
    bool supportLanguage(const QString& languageCodeISO639) const override final;

};

#endif // THETVDBSCRAPER_H
