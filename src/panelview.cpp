#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>
#include <QPushButton>

#include "scrapers/themoviedbscraper.h"
#include "scrapers/allocinescraper.h"
#include "scrapers/thetvdbscraper.h"
#include "searchscraperdialog.h"
#include "promise.h"
#include "scanner/mediainfoscanner.h"
#include "engine/engine.h"
#include "./inprogressdialog.h"

static QPixmap createDefaultPoster(int w, int h){
    QPixmap result(w,h);
    result.fill(Qt::white);

    QPainter pixPaint(&result);

    QPixmap icon;
    icon.load(":/DownloadIcon.png");
    pixPaint.drawPixmap((w-icon.width())/2,(h-icon.height())/2,icon.width(),icon.height(),icon);

    return result;
}


PanelView::PanelView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelView)
{
    ui->setupUi(this);

    Scraper* s=new TheMovieDBScraper(this);
    QAction* tmdbAction = new QAction(s->getIcon(),s->getName(), this);
    tmdbAction->setData(qVariantFromValue((void*)s));

    connect(s, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(s, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(s);

    Scraper* allocine=new AlloCineScraper(this);
    QAction* tmdbActionAllocine = new QAction(s->getIcon(),s->getName(), this);
    tmdbActionAllocine->setData(qVariantFromValue((void*)s));

    connect(allocine, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(allocine, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(allocine);

    Scraper* tvdb=new TheTVDBScraper(this);
    QAction* tvdbAction = new QAction(tvdb->getIcon(),tvdb->getName(), this);
    tvdbAction->setData(qVariantFromValue((void*)s));

    connect(tvdb, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(tvdb, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(tvdb);

    scene = new QGraphicsScene(this);

    setDir();

}

PanelView::~PanelView()
{
    delete ui;
}

void PanelView::enableCastRemove(){
    if(ui->castListWidget->currentItem() && ui->castListWidget->currentItem()->isSelected() == true)
         ui->actionCastRemove->setEnabled(true);
     else
         ui->actionCastRemove->setEnabled(false);
}

void PanelView::castRemove(){
    qDeleteAll(ui->castListWidget->selectedItems());

    QStringList castList;
    for(int i = 0 ; i < ui->castListWidget->count() ; i++)
    {
       QListWidgetItem *item = ui->castListWidget->item(i);
       if (item){
           castList << item->text();
       }
    }

    setCast(castList);
    rebuildTemplate();
}

void PanelView::setProceeded(Engine* engine, const QModelIndex &index){

    ui->stackedWidget->setCurrentIndex(3);

    QObject::disconnect(engine,SIGNAL(previewOK(QGraphicsScene* )),this,0);
    QObject::connect(engine, &Engine::previewOK, [=](QGraphicsScene* newScene){
        ui->graphicsViewBingo->setScene(newScene);
    });

    engine->preview(index);

}

void PanelView::setDir(){
    ui->stackedWidget->setCurrentIndex(2);
}

void PanelView::setProceedable(Engine* engine, const QModelIndex &index){

    ui->stackedWidget->setCurrentIndex(0);

    ui->Proceed->disconnect();

    QObject::connect(ui->Proceed, &QToolButton::released, [=]()
    {
        search(engine, index);
    });
}

void PanelView::search(Engine* engine, const QModelIndex &index){

    currentSearch = MediaSearch();

    currentSearch.engine=engine;

    if (engine->canGiveFileInfo()){
        QFileInfo f=engine->getFileInfo(index);
        currentSearch.fileInfo=f;
        QVariant fileInfo;
        fileInfo.setValue(f);
        currentSearch.texts[Template::Properties::fileinfo]=fileInfo;

        MediaInfoScanner ff;
        Scanner::AnalysisResult r=ff.analyze(f);

        QVariant mediaInfo;
        mediaInfo.setValue(r.mediaInfo);
        currentSearch.texts[Template::Properties::mediainfo]=mediaInfo;


        SearchScraperDialog fd(this, f , this->scrapes, &this->manager);
        if (fd.exec()==QDialog::Accepted){
            if (!fd.getResult().isNull()){
                currentSearch.fd=fd.getResult();
                if (!fd.getResult().isTV()){
                    fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode());
                } else {
                    fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode());
                }
            }
        }
    }
}
void PanelView::foundEpisode(const Scraper* scraper,SearchEpisodeInfo b){

    currentSearch.texts.clear();

    currentSearch.texts[Template::Properties::title]=b.title;
    currentSearch.texts[Template::Properties::originaltitle]=b.originalTitle;
    currentSearch.texts[Template::Properties::tv]=QVariant(true);
    currentSearch.texts[Template::Properties::season]=QVariant(b.season);
    currentSearch.texts[Template::Properties::episode]=QVariant(b.episode);
    currentSearch.texts[Template::Properties::episodetitle]=b.episodeTitle;
    currentSearch.texts[Template::Properties::network]=b.network;

    if (b.productionYear>1900){
        currentSearch.texts[Template::Properties::year]=QString::number(b.productionYear);
    }

    if (b.runtime>0){
        currentSearch.texts[Template::Properties::runtime]=QDateTime::fromTime_t(b.runtime).toUTC().toString("h'H 'mm");
    }

    if (b.rating>0. && b.rating<=10.){
        currentSearch.texts[Template::Properties::rating]=QString::number(b.rating,'f',1);
    }

    ui->toolButtonRescrap->setIcon(scraper->getIcon());

    ui->labelEpisodeTitle->setVisible(true);
    ui->labelSeasonEpisode->setVisible(true);

    ui->stackedWidget->setCurrentIndex(1);
    ui->synopsis->setText(b.synopsis);

    ui->castListWidget->clear();
    ui->castListWidget->addItems(b.actors);

    ui->directorlistWidget->clear();
    for (const QString& director : b.directors){
        ui->directorlistWidget->addItem(director);
    }

    /*ui->castToolButton->disconnect();
    QObject::connect(ui->castToolButton, &QPushButton::released, [=]()
    {
        setCast(b.actors);
        rebuildTemplate();
    });
*/
  /*   ui->toolButtonSysnopsis->disconnect();

 QObject::connect(ui->toolButtonSysnopsis, &QPushButton::released, [=]()
    {
        setSynopsis(b.synopsis);
        rebuildTemplate();
    });*/

    ui->labelEpisodeTitle->setText(b.title);

    // ui->labelSeasonEpisode->setText(QString("Season %1 - Episode %2").arg(b.season).arg(b.episode));*/

    scene->clear();

    ui->graphicsViewPosters->setScene(nullptr);

    int x=0;
    int y=20;
    int w=200;
    int h=200;

    QSet<QString> urls;

    addImages( urls, x,  y, w, h, scene, scraper,  manager, b.postersHref, b.postersSize,Scraper::ImageType::POSTER);
    addImages( urls, x,  y, w, h, scene, scraper,  manager, b.backdropsHref, b.backdropsSize,Scraper::ImageType::BACKDROP);

    if (!b.bannersHref.isEmpty()){
        foreach (const QString& url , b.bannersHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(),QSize(w,h), Qt::KeepAspectRatio,Scraper::ImageType::BANNER);

            qDebug()<< realUrl;

            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            this->addRequest(manager,realUrl, pi,  x,  y,  w,  h);

            QPushButton* b = new QPushButton("Plus");

            QObject::connect(b, &QPushButton::released, [=]()
            {
                setBanner(url,scraper);
            });

            QGraphicsProxyWidget* button = scene->addWidget(b);
            button->setPos(x,h);

            x+=w+10;

        }
    }

    ui->graphicsViewPosters->setScene(scene);

    rebuildTemplate(true);
}


#include <QTime>

void delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void PanelView::addImages( QSet<QString>& urls, int& x, int& y, int& w, int& h, QGraphicsScene* scene, const Scraper* scraper, QNetworkAccessManager& manager, const QStringList&  hrefs, const QList<QSize>& sizes, const Scraper::ImageType type){
    for (int i=0; i<hrefs.size(); i++){

        QString url= hrefs[i];

        QSize posterSize;
        if (sizes.size()>i){
            posterSize=sizes[i];
        }

        QString realUrl=scraper->getBestImageUrl(url,posterSize,QSize(w,h));
        if (urls.contains(realUrl)){
            continue;
        }

        urls.insert(realUrl);

        scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

        QPixmap scaled = createDefaultPoster(w,h);

        QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
        pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

        addRequest(manager,realUrl, pi,  x,  y,  w,  h);

        QPushButton* b = new QPushButton("Plus");

        if (type==Scraper::ImageType::POSTER){
            QObject::connect(b, &QPushButton::released, [=]()
            {
                setPoster(url,posterSize,scraper);
            });
        } else  if (type==Scraper::ImageType::BACKDROP) {
            QObject::connect(b, &QPushButton::released, [=]()
            {
                setBackdrop(url,posterSize,scraper);
            });
        }

        QGraphicsProxyWidget* button = scene->addWidget(b);
        button->setPos(x,h);

        x+=w+10;
    }
}

QList<PanelView::M_M> PanelView::urls;
Promise* PanelView::currentPromise=nullptr;

void PanelView::startPromise( QNetworkAccessManager* manager){
    if (urls.isEmpty() || currentPromise != nullptr){
        return;
    }

    M_M url=urls.takeFirst();
    currentPromise=Promise::loadAsync(*manager,url.url,false,false,QNetworkRequest::LowPriority);

    QObject::connect(currentPromise, &Promise::completed, [=]()
    {
        if (currentPromise->reply->error() ==QNetworkReply::NoError){
            QByteArray qb=currentPromise->reply->readAll();
            setImageFromInternet(qb,url.itemToUpdate,url.x,url.y,url.w,url.h);

        } else {
            qDebug() << currentPromise->reply->errorString();
        }

        currentPromise=nullptr;
        startPromise(manager);
    });

}

void PanelView::addRequest(QNetworkAccessManager & manager, const QString& url,  QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h){
    M_M f;
    f.h=h;
    f.itemToUpdate=itemToUpdate;
    f.url=url;
    f.w=w;
    f.x=x;
    f.y=y;
    urls.append(f);
    startPromise(&manager);
}


void PanelView::foundMovie(const Scraper* scraper,SearchMovieInfo b){

    urls.clear();

    currentSearch.texts[Template::Properties::title]=b.title;
    currentSearch.texts[Template::Properties::originaltitle]=b.originalTitle;
    currentSearch.texts[Template::Properties::tv]=QVariant(false);

    if (b.productionYear>1900){
        currentSearch.texts[Template::Properties::year]=b.productionYear;
    }

    if (b.runtime>0){
        currentSearch.texts[Template::Properties::runtime]=QDateTime::fromTime_t(b.runtime).toUTC().toString("h'H 'mm");
    }

    if (b.rating>0. && b.rating<=10.){
        currentSearch.texts[Template::Properties::rating]=QString::number(b.rating,'f',1);
    }

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

    ui->castListWidget->clear();
    for (const QString& actor : b.actors){
        ui->castListWidget->addItem(actor);
    }

    ui->directorlistWidget->clear();
    for (const QString& director : b.directors){
        ui->directorlistWidget->addItem(director);
    }
    /*ui->castToolButton->disconnect();
    QObject::connect(ui->castToolButton, &QPushButton::released, [=]()
    {
        setCast(b.actors);
        rebuildTemplate();
    });
*/
    //  ui->directorLineEdit->setText(b.directors);


 /*   ui->toolButtonSysnopsis->disconnect();
    QObject::connect(ui->toolButtonSysnopsis, &QPushButton::released, [=]()
    {
        setSynopsis(b.synopsis);
        rebuildTemplate();
    });
*/
    ui->directorToolButton->disconnect();
    QObject::connect(ui->directorToolButton, &QPushButton::released, [=]()
    {
        setDirectors(b.directors);
        rebuildTemplate();
    });
    scene->clear();

    ui->graphicsViewPosters->setScene(nullptr);

    int x=0;
    int y=20;
    int w=200;
    int h=200;

    QSet<QString> urls;

    addImages( urls, x,  y, w, h, scene, scraper,  manager, b.postersHref, b.postersSize,Scraper::ImageType::POSTER);
    addImages( urls, x,  y, w, h, scene, scraper,  manager, b.backdropsHref, b.backdropsSize,Scraper::ImageType::BACKDROP);

    ui->graphicsViewPosters->setScene(scene);

    rebuildTemplate(true);
}


void  PanelView::enableSynopsis(bool enable){
    this->ui->synopsis->setReadOnly(!enable);

    if (!enable){
        setSynopsis(this->ui->synopsis->toPlainText());
        rebuildTemplate();
    }
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


void PanelView::setBanner(const QString& url, const Scraper *_currentScrape){
    currentSearch._banner=ScraperResource(url,QSize(),_currentScrape);

    if (!currentSearch._banner.resources.isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._banner.scraper->getBestImageUrl(currentSearch._banner.resources,QSize(),currentSearch.engine->getPosterSize(), Qt::KeepAspectRatio,Scraper::ImageType::BANNER);

        Promise* promise=Promise::loadAsync(manager,url,false);

        //  QObject::connect(this, &templateYadis::canceled, promise, &Promise::canceled);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->reply->error() ==QNetworkReply::NoError){
                QByteArray qb=promise->reply->readAll();
                QPixmap bannerPixmap;
                if (bannerPixmap.loadFromData(qb)){
                    setBannerState(NETRESOURCE::OK, bannerPixmap);
                }else {
                    setBannerState(NETRESOURCE::ERROR);
                }
            } else if (promise->reply->error() ==QNetworkReply::OperationCanceledError){
                setBannerState(NETRESOURCE::CANCELED);
            } else {
                setBannerState(NETRESOURCE::ERROR);
            }

            p->closeAndDeleteLater();

        });

    } else {
        setPosterState(NETRESOURCE::NONE);
    }

}
void PanelView::setPoster (const QString& url, const QSize& originalSize, const Scraper *_currentScrape){

    currentSearch._poster=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._poster.resources.isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._poster.scraper->getBestImageUrl(currentSearch._poster.resources,originalSize,currentSearch.engine->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false,QNetworkRequest::Priority::HighPriority);

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

            p->closeAndDeleteLater();
        });

    } else {
        setPosterState(NETRESOURCE::NONE);
    }

}

void PanelView::setBackdropState(PanelView::NETRESOURCE backdropState, const QPixmap& backDrop){

    bool same=currentSearch.backdropState == backdropState;
    if (same){
        QPixmap currentBackdrop=currentSearch.texts[Template::Properties::backdrop].value<QPixmap>();
        same = backDrop.toImage()==currentBackdrop.toImage();
    }
    if (!same){
        currentSearch.texts[Template::Properties::backdrop]=backDrop;
        rebuildTemplate();
    }
}

void PanelView::setPosterState(PanelView::NETRESOURCE posterState, const QPixmap& poster){
    bool same=currentSearch.posterState != posterState;
    if (same){
        QPixmap currentPoster=currentSearch.texts[Template::Properties::poster].value<QPixmap>();
        same = poster.toImage()==currentPoster.toImage();
    }
    if (!same){
        currentSearch.texts[Template::Properties::poster]=poster;
        rebuildTemplate();
    }
}

void PanelView::setBannerState(PanelView::NETRESOURCE bannerState, const QPixmap& banner){
    bool same=currentSearch.bannerState != bannerState;
    if (same){
        QPixmap currentBanner=currentSearch.texts[Template::Properties::banner].value<QPixmap>();
        same = banner.toImage()==currentBanner.toImage();
    }
    if (!same){
        currentSearch.texts[Template::Properties::banner]=banner;
        rebuildTemplate();
    }
}
void PanelView::setBackdrop(const QString& url, const QSize& originalSize,const Scraper *_currentScrape){

    currentSearch._backdrop=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._backdrop.resources.isEmpty()){
        QString url=currentSearch._backdrop.scraper->getBestImageUrl(currentSearch._backdrop.resources,originalSize,currentSearch.engine->getBackdropSize(),Qt::KeepAspectRatioByExpanding);

        InProgressDialog* p=InProgressDialog::create();

        Promise* promise=Promise::loadAsync(manager,url,false);

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

            p->closeAndDeleteLater();
        });

    } else {
        setBackdropState(NETRESOURCE::NONE);
    }

}

void PanelView::rebuildTemplate(bool reset) {
    disconnect(SIGNAL(previewOK(QGraphicsScene* )));
    connect(currentSearch.engine, SIGNAL(previewOK(QGraphicsScene* )), this, SLOT(previewOK(QGraphicsScene*  )));
    currentSearch.engine->preview(currentSearch.texts,reset);
}

void PanelView::previewOK(QGraphicsScene* s){
    ui->graphicsView->setScene(s);
}

void PanelView::setSynopsis(const QString& synopsis){
    currentSearch.texts[Template::Properties::synopsis]=synopsis;
}

void PanelView::setCast(const QStringList& actors){
    currentSearch.texts[Template::Properties::actors]=actors;
}

void PanelView::setDirectors(const QStringList& directors){
    currentSearch.texts[Template::Properties::director]=directors;
}

void PanelView::proceed(){
    currentSearch.engine->proceed();
}

void PanelView::rescrap() {

    SearchScraperDialog fd(this, currentSearch.fd , this->scrapes, &this->manager);
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
