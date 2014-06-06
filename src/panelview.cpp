#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>

#include "scrapers/themoviedbscraper.h"
#include "searchscraperdialog.h"
#include "promise.h"
#include "scanner/mediainfoscanner.h"

PanelView::PanelView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelView)
{
    ui->setupUi(this);

    Scraper* s=new TheMovieDBScraper;
    QAction* tmdbAction = new QAction(s->getIcon(),"&TMDB", this);
    tmdbAction->setData(qVariantFromValue((void*)s));

    connect(s, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(s, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(s);

    scene = new QGraphicsScene(this);

    setDir();

#ifdef Q_OS_WIN32
    b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");
#else
    b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    // b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");

    // b.loadTemplate("/home/teddy/Developpement/Maxx Shiny/template.xml");
#endif

    connect(&b, SIGNAL(tivxOk(QPixmap )), this, SLOT(resultOk(QPixmap )));

}

PanelView::~PanelView()
{
    delete ui;
}

void PanelView::setProceeded(QGraphicsScene* s){
    ui->graphicsViewBingo->setScene(s);
    ui->stackedWidget->setCurrentIndex(3);
}

void PanelView::setDir(){
    ui->stackedWidget->setCurrentIndex(2);
}

void PanelView::setProceedable(const QFileInfo& fileInfo){
    ui->stackedWidget->setCurrentIndex(0);

    ui->Proceed->disconnect();

    QObject::connect(ui->Proceed, &QPushButton::released, [=]()
    {
        search(fileInfo);
    });
}

void PanelView::search(QFileInfo f){

    currentSearch = MediaSearch();

    currentSearch.fileInfo=f;

    MediaInfoScanner ff;
    Scanner::AnalysisResult r=ff.analyze(f);

    currentSearch.mediaInfo=r.mediaInfo;



    SearchScraperDialog fd(this, f , this->scrapes, &this->manager);
    if (fd.exec()==QDialog::Accepted){
        if (!fd.getResult().isNull()){
            if (!fd.getResult().isTV()){
                fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode());
            } else {
                fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode());
            }
        }
    }

}
void PanelView::foundEpisode(const Scraper* scraper,SearchEpisodeInfo b){

    /*  ui->toolButtonRescrap->setIcon(scraper->getIcon());

    ui->labelEpisodeTitle->setVisible(true);
    ui->labelSeasonEpisode->setVisible(true);

    ui->stackedWidget->setCurrentIndex(1);
    ui->synopsis->setText(b.synopsis);


    ui->labelEpisodeTitle->setText(b.title);

    ui->labelSeasonEpisode->setText(QString("Season %1 - Episode %2").arg(b.season).arg(b.episode));*/
}

static QPixmap createDefaultPoster(int w, int h){
    QPixmap result(w,h);
    result.fill(Qt::white);

    QPainter pixPaint(&result);

    QPixmap icon;
    icon.load(":/DownloadIcon.png");
    pixPaint.drawPixmap((w-icon.width())/2,(h-icon.height())/2,icon.width(),icon.height(),icon);

    return result;
}

void PanelView::foundMovie(const Scraper* scraper,SearchMovieInfo b){
    ui->toolButtonRescrap->setIcon(scraper->getIcon());

    ui->labelEpisodeTitle->setVisible(false);
    ui->labelSeasonEpisode->setVisible(false);

    if (!b.linkHref.isEmpty()){
        ui->labelMovieTitle->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
        ui->labelMovieTitle->setTextFormat(Qt::RichText);
        ui->labelMovieTitle->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelMovieTitle->setOpenExternalLinks(true);
    } else {
        ui->labelMovieTitle->setText(b.title);
    }

    ui->stackedWidget->setCurrentIndex(1);
    if (!b.linkHref.isEmpty()){
        ui->labelUrl->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
        ui->labelUrl->setTextFormat(Qt::RichText);
        ui->labelUrl->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelUrl->setOpenExternalLinks(true);
    }

    ui->synopsis->setText(b.synopsis);
    /*
    ui->toolButtonSysnopsis->disconnect();
    QObject::connect(ui->toolButtonSysnopsis, &QPushButton::released, [=]()
    {
        setMovieInfo(b);
    });
*/
    scene->clear();

    ui->graphicsViewPosters->setScene(nullptr);

    int x=0;
    int y=20;
    int w=200;
    int h=200;

    QSet<QString> urls;

    if (!b.postersHref.isEmpty()){
        foreach (const QString& url , b.postersHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(w,h));
            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            Promise* promise=Promise::loadAsync(manager,realUrl);
            QObject::connect(promise, &Promise::completed, [=]()
            {
                if (promise->reply->error() ==QNetworkReply::NoError){
                    QByteArray qb=promise->reply->readAll();
                    setImageFromInternet( qb, pi,  x,  y,  w,  h);
                }
            });

            QPushButton* b = new QPushButton("Plus");

            QObject::connect(b, &QPushButton::released, [=]()
            {
                setPoster(url,scraper);
            });

            QGraphicsProxyWidget* button = scene->addWidget(b);
            button->setPos(x,h);

            x+=w+10;

        }
    }

    if (!b.backdropsHref.isEmpty()){
        foreach (const QString& url , b.backdropsHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(w,h));
            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            Promise* promise=Promise::loadAsync(manager,realUrl);
            QObject::connect(promise, &Promise::completed, [=]()
            {
                if (promise->reply->error() ==QNetworkReply::NoError){
                    QByteArray qb=promise->reply->readAll();
                    setImageFromInternet( qb, pi,  x,  y,  w,  h);
                }
            });

            QPushButton* b = new QPushButton("Plus");

            QObject::connect(b, &QPushButton::released, [=]()
            {
                setBackdrop(url,scraper);
            });

            QGraphicsProxyWidget* button = scene->addWidget(b);

            button->setPos(x,h);

            x+=w+10;

        }
    }

    ui->graphicsViewPosters->setScene(scene);


    rebuildTemplate();

}

void PanelView::setImageFromInternet( QByteArray& qb, QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h){

    if (itemToUpdate==nullptr){
        // Nothing to do
        return;
    } else {
        bool itemSeemsToExistsAnymore=false;
        QList<QGraphicsItem*> all = this->scene->items();
        for (int i = 0; i < all.size(); i++)
        {
            if (itemToUpdate==all[i]){
                itemSeemsToExistsAnymore=true;
                break;
            }
        }

        if (!itemSeemsToExistsAnymore){
            // Nothing to do
            return;
        }
    }

    QPixmap pixmap;
    pixmap.loadFromData(qb);
    QPixmap scaled = (pixmap.width()>w || pixmap.height()>h) ? pixmap.scaled(w,h,Qt::KeepAspectRatio):pixmap;
    itemToUpdate->setPixmap(scaled);
    itemToUpdate->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);
}

void PanelView::setPoster (const QString& url, const Scraper *_currentScrape){

    currentSearch._poster=ScraperResource(url,_currentScrape);

    if (!currentSearch._poster.resources.isEmpty()){
        QString url=currentSearch._poster.scraper->getBestImageUrl(currentSearch._poster.resources,b.getSize());

        Promise* promise=Promise::loadAsync(manager,url);

        //  QObject::connect(this, &templateYadis::canceled, promise, &Promise::canceled);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->reply->error() ==QNetworkReply::NoError){
                QByteArray qb=promise->reply->readAll();
                QPixmap fanArtPixmap;
                if (fanArtPixmap.loadFromData(qb)){
                     setPosterState(NETRESOURCE::OK, fanArtPixmap);
                }else {
                    setPosterState(NETRESOURCE::ERROR);
                }
            } else if (promise->reply->error() ==QNetworkReply::OperationCanceledError){
                setPosterState(NETRESOURCE::CANCELED);
            } else {
                setPosterState(NETRESOURCE::ERROR);
            }
        });

    } else {
        setPosterState(NETRESOURCE::NONE);
    }

}

void PanelView::setBackdropState(PanelView::NETRESOURCE backdropState, const QPixmap& backDrop){
    if (currentSearch.backdropState != backdropState || !(backDrop.toImage()==currentSearch.backdrop.toImage())){
        currentSearch.backdropState=backdropState;
        currentSearch.backdrop=backDrop;
        rebuildTemplate();
    }
}

void PanelView::setPosterState(PanelView::NETRESOURCE posterState, const QPixmap& poster){
    if (currentSearch.posterState != posterState || !(poster.toImage()==currentSearch.poster.toImage())){
        currentSearch.posterState=posterState;
        currentSearch.poster=poster;
        rebuildTemplate();
    }
}

void PanelView::setBackdrop(const QString& url, const Scraper *_currentScrape){

    currentSearch._backdrop=ScraperResource(url,_currentScrape);

    if (!currentSearch._backdrop.resources.isEmpty()){
        QString url=currentSearch._backdrop.scraper->getBestImageUrl(currentSearch._backdrop.resources,b.getSize());

        Promise* promise=Promise::loadAsync(manager,url);

        //  QObject::connect(this, &templateYadis::canceled, promise, &Promise::canceled);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->reply->error() ==QNetworkReply::NoError){
                QByteArray qb=promise->reply->readAll();
                QPixmap fanArtPixmap;
                if (fanArtPixmap.loadFromData(qb)){
                     setBackdropState(NETRESOURCE::OK, fanArtPixmap);
                }else {
                    setBackdropState(NETRESOURCE::ERROR);
                }
            } else if (promise->reply->error() ==QNetworkReply::OperationCanceledError){
                setBackdropState(NETRESOURCE::CANCELED);
            } else {
                setBackdropState(NETRESOURCE::ERROR);
            }
        });

    } else {
        setBackdropState(NETRESOURCE::NONE);
    }

}

void PanelView::rebuildTemplate() {
   b.create(currentSearch.poster, currentSearch.backdrop,currentSearch.texts, currentSearch.mediaInfo);
}

void PanelView::resultOk(QPixmap result){
    ui->labelPoster->setPixmap(result);
}
