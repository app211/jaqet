#ifndef MEDIACHOOSERMEDIATYPE
#define MEDIACHOOSERMEDIATYPE

#include <QLoggingCategory>
#include <QUrl>
#include "scrapers/scraper.h"

enum class ImageType {
    None = 0,
    Banner = 1,
    Poster = 2,
    Backdrop = 4,
    Thumbnail = 8,
    All=Banner | Poster | Backdrop | Thumbnail
};

class MediaChoosed {
    const QUrl _url;
    const QString _localFilePath;
    const Scraper * const _scraper=nullptr;
    const QString _scraperResourceId;

public:
    MediaChoosed(const QUrl& url);
    MediaChoosed(const QString& localFilePath);
    MediaChoosed(const Scraper *scraper, const QString& scraperResourceId);

    QUrl url() const;
    QString localFilePath()  const;
    const Scraper *scraper()  const;
    QString scraperResourceId()  const;

    bool isMediaUrl()  const;
    bool isMediaLocalFilePath()  const;
    bool isMediaScraper() const;
};

Q_DECLARE_LOGGING_CATEGORY(mediaChooser)

#endif // MEDIACHOOSERMEDIATYPE

