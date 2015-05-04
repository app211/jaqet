#ifndef MEDIACHOOSERMEDIATYPE
#define MEDIACHOOSERMEDIATYPE

#include <QLoggingCategory>
#include <QUrl>
#include "scrapers/scraper.h"



class MediaChoosed {
    const QUrl _url;
    const QString _localFilePath;
    const ScraperResource _scraperResource;
public:
    MediaChoosed(const QUrl& url);
    MediaChoosed(const QString& localFilePath);
    MediaChoosed(const ScraperResource& scraperResource);

    QUrl url() const;
    QString localFilePath()  const;
    ScraperResource scraperResource()  const;

    bool isMediaUrl()  const;
    bool isMediaLocalFilePath()  const;
    bool isMediaScraper() const;
};

Q_DECLARE_LOGGING_CATEGORY(mediaChooser)

#endif // MEDIACHOOSERMEDIATYPE

