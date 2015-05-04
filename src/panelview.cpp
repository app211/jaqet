#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>
#include <QGraphicsObject>
#include <QPushButton>
#include <QMovie>

#include "scrapers/themoviedbscraper.h"
#include "scrapers/allocinescraper.h"
#include "scrapers/thetvdbscraper.h"
#include "scrapers/defaultscraper.h"
#include "searchscraperdialog.h"
#include "promise.h"
#include "scanner/mediainfoscanner.h"
#include "engine/engine.h"
#include "./inprogressdialog.h"
#include "mediachooserpopup.h"

MediaChooserPopup* c;

PanelView::PanelView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelView)
{
    ui->setupUi(this);

    Scraper* defaultScraper=new DefaultScraper(this);
    QAction* tmdbActionDefault = new QAction(defaultScraper->getIcon(),defaultScraper->getName(), this);
    tmdbActionDefault->setData(qVariantFromValue((void*)defaultScraper));

    connect(defaultScraper, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(defaultScraper, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(defaultScraper);

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

    setDir();

    c=new MediaChooserPopup(this);

    ui->chooseBackgroundButton->setPopup(c,ImageType::Backdrop);
    ui->choosePosterButton->setPopup(c,ImageType::Poster);
    ui->chooseThumbailButton->setPopup(c,ImageType::Thumbnail);
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


void PanelView::enableDirectorRemove(){
    if(ui->directorListWidget->currentItem() && ui->directorListWidget->currentItem()->isSelected() == true)
        ui->directorListWidget->setEnabled(true);
    else
        ui->directorListWidget->setEnabled(false);
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

void PanelView::directorRemove(){
    qDeleteAll(ui->directorListWidget->selectedItems());

    QStringList directorsList;
    for(int i = 0 ; i < ui->directorListWidget->count() ; i++)
    {
        QListWidgetItem *item = ui->directorListWidget->item(i);
        if (item){
            directorsList << item->text();
        }
    }

    this->setDirectors(directorsList);
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
                    fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode(), Scraper::SearchOption::All);
                } else {
                    fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode(), Scraper::SearchOption::All);
                }
            }
        }
    }
}

void PanelView::foundEpisode(const Scraper* scraper,SearchEpisodeInfo b){


    currentSearch.texts[Template::Properties::title]=b.title;
    currentSearch.texts[Template::Properties::originaltitle]=b.originalTitle;
    currentSearch.texts[Template::Properties::tv]=QVariant(true);
    currentSearch.texts[Template::Properties::season]=QVariant(b.season);
    currentSearch.texts[Template::Properties::episode]=QVariant(b.episode);
    currentSearch.texts[Template::Properties::episodetitle]=b.episodeTitle;
    currentSearch.texts[Template::Properties::network]=b.network;
    currentSearch.texts[Template::Properties::aired]=b.aired;

    if (b.productionYear>1900){
        currentSearch.texts[Template::Properties::year]=QString::number(b.productionYear);
    }

    if (b.runtimeInSec>0){
        currentSearch.texts[Template::Properties::runtime]=b.runtimeInSec;
    }

    if (b.rating>0. && b.rating<=10.){
        currentSearch.texts[Template::Properties::rating]=QString::number(b.rating,'f',1);
    }

    ui->toolButtonRescrap->setIcon(scraper->getIcon());

    ui->labelEpisodeTitle->setVisible(true);
    ui->labelSeasonEpisode->setVisible(true);

    ui->stackedWidget->setCurrentIndex(1);

    if (!ui->checkBoxLockSynopsis->isLock() /*&& currentSearch.texts[Template::Properties::synopsis].isNull()*/){
        ui->synopsis->setText(b.synopsis);
        setSynopsis(b.synopsis);
    }

    if (!ui->checkBoxLockCast->isLock()){
        ui->castListWidget->clear();
        ui->castListWidget->addItems(b.actors);
        setCast(b.actors);
    }

    if (!ui->checkBoxLockDirectors->isLock()){

        ui->directorListWidget->clear();
        ui->directorListWidget->addItems(b.directors);
        setDirectors(b.directors);
    }

    ui->labelEpisodeTitle->setText(b.title);

    if (!ui->checkBoxLockGenre->isLock()){
        ui->genreListWidget->clear();
        ui->genreListWidget->addItems(b.genre);
    }


    c->clear();

    bool backDropSet = false;
    bool thumbnailSet = false;

    if (!this->currentSearch.fd.getPosterHref().isEmpty()){
        addImages(  scraper,  QStringList() << this->currentSearch.fd.getPosterHref(), QList<QSize>(), ImageType::Poster);
        if (!backDropSet){
            //     setBackdrop(this->currentSearch.fd.getPosterHref(),QSize(),scraper);
            backDropSet=true;
        }
    }

    addImages(  scraper,   b.postersHref, b.postersSize,ImageType::Poster);
    if (!thumbnailSet && b.postersHref.size()>0){
        //  this->setThumbnail(b.postersHref.at(0),b.postersSize.size()>0? b.postersSize.at(0) : QSize(),scraper);
        thumbnailSet=true;
    }

    addImages(  scraper,  b.backdropsHref, b.backdropsSize, ImageType::Backdrop);

    if (!backDropSet && b.backdropsHref.size()>0){
        // setBackdrop(b.backdropsHref.at(0),b.backdropsSize.size()>0? b.backdropsSize.at(0) : QSize(),scraper);
        backDropSet=true;
    }

    addImages(  scraper,  b.bannersHref, b.bannersSize,ImageType::Banner);

    addImages(  scraper,  b.thumbailHref, b.thumbailSize,ImageType::Thumbnail);




    rebuildTemplate(true);
}


void PanelView::addImages( const Scraper* scraper, const QStringList&  hrefs, const QList<QSize>& sizes,  QFlags<ImageType> type){

    for (int i=0; i<hrefs.size(); i++){

        QString url= hrefs[i];

        QSize posterSize;
        if (sizes.size()>i){
            posterSize=sizes[i];
        }

        c->addImageFromScraper(scraper,url,posterSize,type);
    }
}





void PanelView::foundMovie(const Scraper* scraper,SearchMovieInfo b){

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
        /*ui->labelUrl->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
        ui->labelUrl->setTextFormat(Qt::RichText);
        ui->labelUrl->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelUrl->setOpenExternalLinks(true);*/
    }

    ui->synopsis->setText(b.synopsis);

    ui->castListWidget->clear();
    for (const QString& actor : b.actors){
        ui->castListWidget->addItem(actor);
    }

    ui->directorListWidget->clear();
    for (const QString& director : b.directors){
        ui->directorListWidget->addItem(director);
    }

    c->clear();

    addImages(  scraper,  b.postersHref, b.postersSize,ImageType::Poster);
    addImages(   scraper,  b.backdropsHref, b.backdropsSize,ImageType::Backdrop);

    rebuildTemplate(true);
}


void  PanelView::enableSynopsis(bool enable){
    this->ui->synopsis->setReadOnly(!enable);

    if (!enable){
        setSynopsis(this->ui->synopsis->toPlainText());
        rebuildTemplate();
    }
}



void PanelView::setBanner(const QString& url, const Scraper *_currentScrape ){
    currentSearch._banner=ScraperResource(url,QSize(),_currentScrape);

    if (!currentSearch._banner.resources().isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._banner.scraper()->getBestImageUrl(currentSearch._banner.resources(),QSize(),currentSearch.engine->getPosterSize(), Qt::KeepAspectRatio,ImageType::Banner);

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

void PanelView::setThumbnail (const QString& url, const QSize& originalSize, const Scraper *_currentScrape){

    currentSearch._thumbnail=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._thumbnail.resources().isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._thumbnail.scraper()->getBestImageUrl(currentSearch._thumbnail.resources(),originalSize,currentSearch.engine->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false,QNetworkRequest::Priority::HighPriority);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->reply->error() ==QNetworkReply::NoError){
                QByteArray qb=promise->reply->readAll();
                QPixmap thumbnail;
                if (thumbnail.loadFromData(qb)){
                    setThumbnailState(NETRESOURCE::OK, thumbnail);
                }else {
                    setThumbnailState(NETRESOURCE::ERROR);
                }
            } else if (promise->reply->error() ==QNetworkReply::OperationCanceledError){
                setThumbnailState(NETRESOURCE::CANCELED);
            } else {
                setThumbnailState(NETRESOURCE::ERROR);
            }

            p->closeAndDeleteLater();
        });

    } else {
        setThumbnailState(NETRESOURCE::NONE);
    }

}


void PanelView::setPoster (const QString& url, const QSize& originalSize, const Scraper *_currentScrape){

    currentSearch._poster=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._poster.resources().isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._poster.scraper()->getBestImageUrl(currentSearch._poster.resources(),originalSize,currentSearch.engine->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false,QNetworkRequest::Priority::HighPriority);

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
        ui->chooseBackgroundButton->setMedia(backDrop);
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
        ui->choosePosterButton->setMedia(poster);
        rebuildTemplate();
    }
}

void PanelView::setThumbnailState(PanelView::NETRESOURCE thumbnailState, const QPixmap& thumbnail){
    bool same=currentSearch.thumbnailState != thumbnailState;
    if (same){
        QPixmap currentThumbnail=currentSearch.texts[Template::Properties::thumbnail].value<QPixmap>();
        same = thumbnail.toImage()==currentThumbnail.toImage();
    }
    if (!same){
        currentSearch.texts[Template::Properties::thumbnail]=thumbnail;
        ui->chooseThumbailButton->setMedia(thumbnail);
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

    if (!currentSearch._backdrop.resources().isEmpty()){
        QString url=currentSearch._backdrop.scraper()->getBestImageUrl(currentSearch._backdrop.resources(),originalSize,currentSearch.engine->getBackdropSize(),Qt::KeepAspectRatioByExpanding);

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
                fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode(), Scraper::SearchOption::Poster);
            } else {
                fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode(),Scraper::SearchOption::Poster);
            }
        }
    }
}


void PanelView::backgroundSelected(const MediaChoosed& mediaChoosed){

    if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        setBackdrop(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}

void PanelView::posterSelected(const MediaChoosed& mediaChoosed){

    if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        this->setPoster(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
     }
}

void PanelView::thumbnailSelected(const MediaChoosed& mediaChoosed){

    if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        this->setThumbnail(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}
