#ifndef PANELVIEW_H
#define PANELVIEW_H

#include <QWidget>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <QPointer>

#include "scrapers/scraper.h"
#include "scanner/scanner.h"
#include "engine/template/template.h"
#include "promise.h"
#include "mediachoosermediatype.h"

class MyProxyModel;
class Engine;
#include <QNetworkAccessManager>

namespace Ui {
class PanelView;
}



class PanelView : public QWidget
{
    Q_OBJECT

public:
    explicit PanelView(QWidget *parent = 0);
    ~PanelView();

    void setProceedable(Engine *engine, const QModelIndex &index);
    void setProceeded(Engine *engine, const QModelIndex &index);
    void setDir();
private:
    enum class NETRESOURCE {
        UNKNOWN, NONE, ERROR, OK, CANCELED
    };

    struct MediaSearch {
        Engine* engine;
        ScraperResource _poster;
        NETRESOURCE posterState=NETRESOURCE::UNKNOWN;

        ScraperResource _backdrop;
        NETRESOURCE backdropState=NETRESOURCE::UNKNOWN;

        ScraperResource _banner;
        NETRESOURCE bannerState=NETRESOURCE::UNKNOWN;

        ScraperResource _thumbnail;
        NETRESOURCE thumbnailState=NETRESOURCE::UNKNOWN;

        QFileInfo fileInfo;
        MediaInfo mediaInfo;
        QMap<Template::Properties, QVariant> texts;
        FoundResult fd;
    };


    Ui::PanelView *ui;

    QNetworkAccessManager manager;
    QList<Scraper*> scrapes;
    MediaSearch currentSearch;



    void setBackdropState(NETRESOURCE fa, const QPixmap &backDrop=QPixmap());
    void setPosterState(NETRESOURCE posterState, const QPixmap& poster=QPixmap());
    void setBannerState(NETRESOURCE bannerState, const QPixmap& banner=QPixmap());
    void setThumbnailState(NETRESOURCE thumbnailState, const QPixmap& thumbnail=QPixmap());
    void rebuildTemplate(bool reset=false);
    void setSynopsis(const QString& synopsis);
    void setCast(const QStringList& actors);
    void setDirectors(const QStringList& directors);
    void addImages(const Scraper* scraper, const QStringList&  hrefs, const QList<QSize>& sizes, QFlags<ImageType> type);

private slots:

    void search(Engine *engine, const QModelIndex &index);

    void foundMovie(const Scraper *scraper, SearchMovieInfo b);
    void foundEpisode(const Scraper *scraper, SearchEpisodeInfo b);

    void setPoster (const QString& url, const QSize &originalSize, const Scraper *scrape );
    void setThumbnail (const QString& url, const QSize &originalSize, const Scraper *scrape );
    void setBackdrop(const QString& url, const QSize& originalSize, const Scraper *scrape);
    void setBanner(const QString& url, const Scraper *scrape);
    void proceed();
    void rescrap();
    void enableCastRemove();
    void castRemove();

    void enableSynopsis(bool enable);

    void enableDirectorRemove();
    void directorRemove();

    // From Engine
    void previewOK(QGraphicsScene*);


    void backgroundSelected(const MediaChoosed& mediaChoosed);
    void posterSelected(const MediaChoosed& mediaChoosed);
    void thumbnailSelected(const MediaChoosed& mediaChoosed);


};

#endif // PANELVIEW_H
