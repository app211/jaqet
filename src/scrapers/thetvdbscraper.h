#ifndef THETVDBSCRAPER_H
#define THETVDBSCRAPER_H

#include "scraper.h"

class TheTVDBScraper : public Scraper
{
    static const QString API_KEY;
    static const uchar icon_png[];

      // Cf. http://thetvdb.com/wiki/index.php?title=Programmers_API
    QList<QString> m_xmlmirrors;
    QList<QString> m_bannermirrors;
    QList<QString> m_zipmirrors;
    bool retrieveMirror = true;
private:
    void parseSeriesList( QNetworkAccessManager* manager, const QString& toSearch, const QByteArray& data, const QString& language);
    void parseMirrorList( const QByteArray& data);
    QString getXMLURL() const ;
    QString getZIPURL() const ;
       QString getBannerURL() const ;
public:
    TheTVDBScraper();
    QIcon getIcon()const;
    QString getName() const;

    QString createURL(const QString& , const QMap<QString, QString>& params) const;
    void findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const;

    void findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const;

    QString getBestImageUrl(const QString& url, const QSize& size) const;

protected:
   void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const;
   void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) ;
   QString postMirrorCreateURL(const QString& , const QMap<QString, QString>& params) const;

  };

#endif // THETVDBSCRAPER_H
