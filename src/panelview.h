#ifndef PANELVIEW_H
#define PANELVIEW_H

#include <QWidget>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "scrapers/scraper.h"
#include "scanner/scanner.h"
#include "engine/template/template.h"

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

        QFileInfo fileInfo;
        MediaInfo mediaInfo;
        QMap<Template::Properties, QVariant> texts;
    };


    Ui::PanelView *ui;

    QNetworkAccessManager manager;
    QList<Scraper*> scrapes;
    MediaSearch currentSearch;

    QGraphicsScene* scene;

    void setBackdropState(NETRESOURCE fa, const QPixmap &backDrop=QPixmap());
    void setPosterState(NETRESOURCE posterState, const QPixmap& poster=QPixmap());
    void rebuildTemplate(bool reset=false);
    void setSynopsis(const QString& synopsis);
    void setCast(const QStringList& actors);
    void setDirectors(const QStringList& directors);

private slots:

    void search(Engine *engine, const QModelIndex &index);

    // From Scrapper
    void foundMovie(const Scraper *scraper, SearchMovieInfo b);
    void foundEpisode(const Scraper *scraper, SearchEpisodeInfo b);

    // From Internet
    void setImageFromInternet( QByteArray& qb, QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h);

    // From UI
    void setPoster (const QString& url, const Scraper *scrape );
    void setBackdrop(const QString& url, const Scraper *scrape);
    void proceed();

    // From Engine
    void previewOK(QGraphicsScene*);

 };

#endif // PANELVIEW_H
