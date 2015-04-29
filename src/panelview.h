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
#include "mygraphicsobject.h"

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

    QGraphicsScene* scene;

    struct M_M {
        QString url;
        QPointer<MyGraphicsObject> itemToUpdate;
        QPointer<QGraphicsProxyWidget> busyIndicator;
        int x;
        int y;
        int w;
        int h;
    };

    static QList<M_M> urls;
    static Promise* currentPromise;

    void setBackdropState(NETRESOURCE fa, const QPixmap &backDrop=QPixmap());
    void setPosterState(NETRESOURCE posterState, const QPixmap& poster=QPixmap());
    void setBannerState(NETRESOURCE bannerState, const QPixmap& banner=QPixmap());
    void setThumbnailState(NETRESOURCE thumbnailState, const QPixmap& thumbnail=QPixmap());
    void rebuildTemplate(bool reset=false);
    void setSynopsis(const QString& synopsis);
    void setCast(const QStringList& actors);
    void setDirectors(const QStringList& directors);
    void addImages( int& x, int& y, int& w, int& h, QGraphicsScene* scene, const Scraper* scraper, QNetworkAccessManager& manager, const QStringList&  hrefs, const QList<QSize>& sizes, const Scraper::ImageType type);

    void addRequest(QNetworkAccessManager & manager, const QString& url,  QPointer<MyGraphicsObject> itemToUpdate, int x, int y, int w, int h, QPointer<QGraphicsProxyWidget> busyIndicator);
    void startPromise( QNetworkAccessManager* manager);

private slots:

    void search(Engine *engine, const QModelIndex &index);

    // From Scrapper
    void foundMovie(const Scraper *scraper, SearchMovieInfo b);
    void foundEpisode(const Scraper *scraper, SearchEpisodeInfo b);

    // From Internet
    void setImageFromInternet(QByteArray& qb, QPointer<MyGraphicsObject> itemToUpdate, int x, int y, int w, int h);

    // From UI
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


};

#endif // PANELVIEW_H
