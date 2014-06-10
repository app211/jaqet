#ifndef PANELVIEW_H
#define PANELVIEW_H

#include <QWidget>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "template/templateyadis.h"
#include "scrapers/scraper.h"
#include "scanner/scanner.h"

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

    void setProceedable(const QFileInfo&);
    void setProceeded(QGraphicsScene*);
    void setDir();
private:
    enum class NETRESOURCE {
        UNKNOWN, NONE, ERROR, OK, CANCELED

    };

    struct MediaSearch {
        ScraperResource _poster;
        NETRESOURCE posterState=NETRESOURCE::UNKNOWN;
        QPixmap poster;

        ScraperResource _backdrop;
        NETRESOURCE backdropState=NETRESOURCE::UNKNOWN;
        QPixmap backdrop;

        QFileInfo fileInfo;
        MediaInfo mediaInfo;
        QMap<QString, QString> texts;
    };


    Ui::PanelView *ui;

    QNetworkAccessManager manager;
    TemplateYadis b;
    QList<Scraper*> scrapes;
    MediaSearch currentSearch;

    QGraphicsScene* scene;

    void setBackdropState(NETRESOURCE fa, const QPixmap &backDrop=QPixmap());
    void setPosterState(NETRESOURCE posterState, const QPixmap& poster=QPixmap());
    void rebuildTemplate();
    void setSynopsis(const QString& synopsis);
    void setCast(const QStringList& actors);
    void setDirectors(const QStringList& directors);

private slots:

    void search(QFileInfo f);

    // From Scrapper
    void foundMovie(const Scraper *scraper, SearchMovieInfo b);
    void foundEpisode(const Scraper *scraper, SearchEpisodeInfo b);

    // From Internet
    void setImageFromInternet( QByteArray& qb, QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h);

    // From UI
    void setPoster (const QString& url, const Scraper *scrape );
    void setBackdrop(const QString& url, const Scraper *scrape);

    // From TemplateEngine
    void resultOk(QPixmap result);

};

#endif // PANELVIEW_H
