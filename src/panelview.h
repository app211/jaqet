#ifndef PANELVIEW_H
#define PANELVIEW_H

#include <QWidget>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QListWidgetItem>
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
    void reset();

private:


    Ui::PanelView *ui;

    QNetworkAccessManager manager;
    QList<Scraper*> scrapes;
    CurrentItemData currentSearch;



    void setBackdropState(NETRESOURCE fa, const QPixmap &backDrop=QPixmap());
    void setPosterState(NETRESOURCE posterState, const QPixmap& poster=QPixmap());
    void setBannerState(NETRESOURCE bannerState, const QPixmap& banner=QPixmap());
    void setThumbnailState(NETRESOURCE thumbnailState, const QPixmap& thumbnail=QPixmap());

    void setSynopsis(const QString& synopsis);
    void setCast(const QStringList& actors);
    void setDirectors(const QStringList& directors);
    void addImages(const Scraper* scraper, const QStringList&  hrefs, const QList<QSize>& sizes, QFlags<ImageType> type);
    void addImages(const Scraper* scraper, const QList<media>& medias,  QFlags<ImageType> type, const QString &language);

    void updateFrom(const CurrentItemData& newData);
    void updateUI();
    void buildPreview( const CurrentItemData& currentSearch);
    void buildPoster( const CurrentItemData& currentSearch);

private slots:

    void search(Engine *engine, const QModelIndex &index);

    void foundMovie(const Scraper *scraper, MediaMovieSearchPtr mediaMovieSearchPtr);
    void foundEpisode(const Scraper *scraper, MediaTVSearchPtr mediaTVSearchPtr);

    void setPoster (const QString& url, const QSize &originalSize, const Scraper *scrape );
    void setThumbnail (const QString& url, const QSize &originalSize, const Scraper *scrape );
    void setBackdrop(const QString& url, const QSize& originalSize, const Scraper *scrape);
    void setBanner(const QString& url,  const QSize& originalSize, const Scraper *scrape);
    void proceed();
    void rescrap();
    void enableCastRemove();
    void castRemove();

    void enableSynopsis(bool enable);

    void enableDirectorRemove();
    void directorRemove();

    void backgroundSelected(const MediaChoosed& mediaChoosed);
    void posterSelected(const MediaChoosed& mediaChoosed);
    void thumbnailSelected(const MediaChoosed& mediaChoosed);
    void bannerSelected(const MediaChoosed& mediaChoosed);

    void on_countriesListWidget_itemChanged(QListWidgetItem *item);
    void on_comboBoxVideoCodec_currentIndexChanged(const QString &arg1);
    void on_comboBoxFormat_currentIndexChanged(const QString &arg1);
};

#endif // PANELVIEW_H
