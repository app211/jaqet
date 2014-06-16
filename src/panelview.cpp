#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>

#include "scrapers/themoviedbscraper.h"
#include "scrapers/allocinescraper.h"
#include "scrapers/thetvdbscraper.h"
#include "searchscraperdialog.h"
#include "promise.h"
#include "scanner/mediainfoscanner.h"
#include "engine/engine.h"

PanelView::PanelView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelView)
{
    ui->setupUi(this);

    Scraper* s=new TheMovieDBScraper;
    QAction* tmdbAction = new QAction(s->getIcon(),s->getName(), this);
    tmdbAction->setData(qVariantFromValue((void*)s));

    connect(s, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(s, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(s);

    Scraper* allocine=new AlloCineScraper;
    QAction* tmdbActionAllocine = new QAction(s->getIcon(),s->getName(), this);
    tmdbActionAllocine->setData(qVariantFromValue((void*)s));

    connect(allocine, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(allocine, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(allocine);

    Scraper* tvdb=new TheTVDBScraper;
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

    QObject::connect(ui->Proceed, &QPushButton::released, [=]()
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
            if (!fd.getResult().isTV()){
                fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode());
            } else {
                fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode());
            }
        }
    }
}
}
void PanelView::foundEpisode(const Scraper* scraper,SearchEpisodeInfo c){

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

#include <QTime>

void delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void PanelView::foundMovie(const Scraper* scraper,SearchMovieInfo b){

    currentSearch.texts[Template::Properties::title]=b.title;
    currentSearch.texts[Template::Properties::originaltitle]=b.originalTitle;

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

    //  ui->directorLineEdit->setText(b.directors);


    ui->toolButtonSysnopsis->disconnect();
    QObject::connect(ui->toolButtonSysnopsis, &QPushButton::released, [=]()
    {
        setSynopsis(b.synopsis);
        rebuildTemplate();
    });


    ui->castToolButton->disconnect();
    QObject::connect(ui->castToolButton, &QPushButton::released, [=]()
    {
        setCast(b.actors);
        rebuildTemplate();
    });

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

    if (!b.postersHref.isEmpty()){
        foreach (const QString& url , b.postersHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(w,h));
            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            qDebug() << realUrl;

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            Promise* promise=Promise::loadAsync(manager,realUrl,false);
            QObject::connect(promise, &Promise::completed, [=]()
            {
                if (promise->reply->error() ==QNetworkReply::NoError){
                    QByteArray qb=promise->reply->readAll();
                    setImageFromInternet( qb, pi,  x,  y,  w,  h);
                } else {
                    qDebug() << promise->reply->errorString();
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

            Promise* promise=Promise::loadAsync(manager,realUrl,false);
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


#include "./inprogressdialog.h"

void PanelView::setPoster (const QString& url, const Scraper *_currentScrape){

    currentSearch._poster=ScraperResource(url,_currentScrape);

    if (!currentSearch._poster.resources.isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._poster.scraper->getBestImageUrl(currentSearch._poster.resources,currentSearch.engine->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false);

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

void PanelView::setBackdrop(const QString& url, const Scraper *_currentScrape){

    currentSearch._backdrop=ScraperResource(url,_currentScrape);

    if (!currentSearch._backdrop.resources.isEmpty()){
        QString url=currentSearch._backdrop.scraper->getBestImageUrl(currentSearch._backdrop.resources,currentSearch.engine->getBackdropSize());

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

void PanelView::rebuildTemplate() {
    disconnect(SIGNAL(previewOK(QGraphicsScene* )));
    connect(currentSearch.engine, SIGNAL(previewOK(QGraphicsScene* )), this, SLOT(previewOK(QGraphicsScene*  )));
    currentSearch.engine->preview(currentSearch.texts);
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
