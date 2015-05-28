#ifndef FANARTTVPROVIDER_H
#define FANARTTVPROVIDER_H

#include "scraper.h"

class TheTVDBScraper;
class TheMovieDBScraper;

class FanArtTVProvider: public Scraper
{
    Q_OBJECT

    QIcon               m_icon;
    TheTVDBScraper*     m_tvdbscraper;
    TheMovieDBScraper*  m_moviedbscraper;
    QString             m_personalApiKey;

    QIcon loadIcon() const;
    bool parseResult(const QJsonDocument& resultset,  MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language, const int season);
    bool parseResult(const QJsonDocument& resultset,  MediaMovieSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language);
    bool isImageForLanguageAndSeason(const QJsonObject& showbackgroundObject, const QString& language, const int season);
    bool isImageForLanguage(const QJsonObject& imageObject, const QString& language);

protected:
    void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const override ;
    void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const override;
    void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) override;
    void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language) override ;

public:
    FanArtTVProvider(QObject *parent=0);

    QIcon icon() const override final;
    QString name() const;

    bool haveCapability(const SearchCapabilities capability) const override final;
    bool supportLanguage(const QString& languageCodeISO639) const override final;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio, QFlags<ImageType> imageType=ImageType::All ) const;

private slots:
    void _found(FilmPrtList films);
    void _found(ShowPtrList shows);
    void _scraperError(const QString& error);
    void _scraperError();

};

#endif // FANARTTVPROVIDER_H
