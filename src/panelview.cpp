#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>
#include <QGraphicsObject>
#include <QPushButton>
#include <QMovie>
#include <QCompleter>

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

    /*Scraper* defaultScraper=new DefaultScraper(this);

    connect(defaultScraper, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(foundMovie(const Scraper*,SearchMovieInfo)));

    connect(defaultScraper, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(foundEpisode(const Scraper*,SearchEpisodeInfo)));

    this->scrapes.append(defaultScraper);
*/
    Scraper* s=new TheMovieDBScraper(this);
    connect(s, SIGNAL(found(const Scraper*, MediaMovieSearchPtr)), this,
            SLOT(foundMovie(const Scraper*,MediaMovieSearchPtr)));

    connect(s, SIGNAL(found(const Scraper*, MediaTVSearchPtr)), this,
            SLOT(foundEpisode(const Scraper*,MediaTVSearchPtr)));

    this->scrapes.append(s);

    Scraper* allocine=new AlloCineScraper(this);

    connect(allocine, SIGNAL(found(const Scraper*, MediaMovieSearchPtr)), this,
            SLOT(foundMovie(const Scraper*,MediaMovieSearchPtr)));

    connect(allocine, SIGNAL(found(const Scraper*, MediaTVSearchPtr )), this,
            SLOT(foundEpisode(const Scraper*,MediaTVSearchPtr )));

    this->scrapes.append(allocine);

    Scraper* tvdb=new TheTVDBScraper(this);

    connect(tvdb, SIGNAL(found(const Scraper*, MediaMovieSearchPtr)), this,
            SLOT(foundMovie(const Scraper*,MediaMovieSearchPtr)));

    connect(tvdb, SIGNAL(found(const Scraper*, MediaTVSearchPtr)), this,
            SLOT(foundEpisode(const Scraper*,MediaTVSearchPtr)));

    this->scrapes.append(tvdb);

    setDir();

    c=new MediaChooserPopup(this);

    ui->chooseBackgroundButton->setPopup(c,ImageType::Backdrop);
    ui->choosePosterButton->setPopup(c,ImageType::Poster);
    ui->chooseThumbailButton->setPopup(c,ImageType::Thumbnail);
    ui->chooseBannerButton->setPopup(c,ImageType::Banner);



    ui->comboResolution->addItems({
                                      QStringLiteral("480i") ,
                                      QStringLiteral("480p"),
                                      QStringLiteral("576i"),
                                      QStringLiteral("576p"),
                                      QStringLiteral("720p"),
                                      QStringLiteral("1080i"),
                                      QStringLiteral("1080p")
                                  });
    /*     QCompleter* resolutionCompleter= new QCompleter(l,this);
         resolutionCompleter->setCaseSensitivity(Qt::CaseInsensitive);
         resolutionCompleter->setCompletionMode(QCompleter::PopupCompletion);

    ui->comboResolution->setCompleter(resolutionCompleter);*/
}

PanelView::~PanelView()
{
    delete ui;
}

static QStringList getItemsListWidget(QListWidget *listWidget, bool onlyNotSelectedItem=false)
{
    QStringList listeItems;

    for(int i = 0 ; i < listWidget->count() ; i++){
        QListWidgetItem *item = listWidget->item(i);
        if (item && ((!onlyNotSelectedItem) || (!item->isSelected()))){
            listeItems << item->text();
        }
    }

    return listeItems;
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

    this->currentSearch.setActors(getItemsListWidget(ui->castListWidget,true));

    updateUI();
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

    // this->setDirectors(directorsList);
    //rebuildTemplate();
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

    connect(engine, SIGNAL(previewOK(QGraphicsScene* )), this, SLOT(previewOK(QGraphicsScene*  )), Qt::UniqueConnection);


    if (engine->canGiveFileInfo()){
        QFileInfo fileInfo=engine->getFileInfo(index);

        MediaInfoScanner ff;
        Scanner::AnalysisResult r=ff.analyze(fileInfo);

        SearchScraperDialog fd(this, fileInfo , this->scrapes, &this->manager);
        if (fd.exec()==QDialog::Accepted){
            if (!fd.getResult().isNull()){
                if (!fd.getResult().isTV()){

                    fd.getResult().getScraper()->findMovieInfo(&this->manager,MediaMovieSearchPtr(new MediaMovieSearch(fileInfo,r.mediaInfo,fd.getResult(),engine)), Scraper::SearchOption::All);
                } else {
                    fd.getResult().getScraper()->findEpisodeInfo(&this->manager,MediaTVSearchPtr(new MediaTVSearch(fileInfo,r.mediaInfo,fd.getResult(),engine)), Scraper::SearchOption::All);
                }
            }
        }
    }
}

void PanelView::updateFrom(const CurrentItemData& newData){
    this->currentSearch=newData;
    updateUI();
}

void PanelView::updateUI(){

    ui->synopsis->setText(currentSearch.synopsis());

    ui->castListWidget->clear();
    ui->castListWidget->addItems(currentSearch.actors());

    ui->directorListWidget->clear();
    ui->directorListWidget->addItems(currentSearch.directors());

    ui->genreListWidget->clear();
    ui->genreListWidget->addItems(currentSearch.genre());

    ui->stackedWidget->setCurrentIndex(1);

    ui->titleEdit->setText(currentSearch.title());

    ui->chooseBackgroundButton->setMedia(currentSearch.currentBackdrop);
    ui->choosePosterButton->setMedia(currentSearch.currentPoster);

    if (currentSearch.isTV()){
        ui->chooseBannerButton->setMedia(currentSearch.currentBanner);
        ui->panelBanner->setVisible(true);
    } else {
        ui->panelBanner->setVisible(false);
    }

    ui->chooseThumbailButton->setMedia(currentSearch.currentThumbail);

    currentSearch.engine()->preview(currentSearch);
}

void PanelView::foundEpisode(const Scraper* scraper, MediaTVSearchPtr mediaTVSearchPtr){

    Q_UNUSED(scraper);

    CurrentItemData newData(mediaTVSearchPtr->engine(),true, mediaTVSearchPtr->mediaInfo(),mediaTVSearchPtr->fileInfo() );

    if (!ui->checkBoxLockSynopsis->isLock()){
        newData.setSynopsis(mediaTVSearchPtr->synopsis());
    } else {
        newData.setSynopsis(ui->synopsis->toPlainText());
    }

    if (!ui->checkBoxLockCast->isLock()){
        newData.setActors(mediaTVSearchPtr->actors());
    } else {
        newData.setActors( getItemsListWidget(ui->castListWidget));
    }

    if (!ui->checkBoxLockDirectors->isLock()){
        newData.setDirectors(mediaTVSearchPtr->directors());
    } else {
        newData.setDirectors( getItemsListWidget(ui->directorListWidget));
    }

    if (!ui->checkBoxLockGenre->isLock()){
        newData.setGenre(mediaTVSearchPtr->genre());
    } else {
        newData.setGenre(getItemsListWidget(ui->genreListWidget));
    }

    if (ui->checkBoxLockBackground->isLock()){
        newData._backdrop=currentSearch._backdrop;
        newData.backdropState=currentSearch.backdropState;
        newData.currentBackdrop=currentSearch.currentBackdrop;
    }

    if (ui->checkBoxLockBanner->isLock()){
        newData._banner=currentSearch._banner;
        newData.bannerState=currentSearch.bannerState;
        newData.currentBanner=currentSearch.currentBanner;
    }

    if (ui->checkBoxLockPoster->isLock()){
        newData._poster=currentSearch._poster;
        newData.posterState=currentSearch.posterState;
        newData.currentPoster=currentSearch.currentPoster;
    }

    if (ui->checkBoxLockThumbail->isLock()){
        newData._thumbnail=currentSearch._thumbnail;
        newData.thumbnailState=currentSearch.thumbnailState;
        newData.currentThumbail=currentSearch.currentThumbail;
    }

    newData.setRuntimeInSec(mediaTVSearchPtr->runtimeInSec());
    newData.setYear(mediaTVSearchPtr->productionYear());
    newData.setTitle(mediaTVSearchPtr->title());
    newData.setEpisodeRating(mediaTVSearchPtr->episodeRating());
    newData.setShowRating(mediaTVSearchPtr->showRating());
    newData.setSeasonRating(mediaTVSearchPtr->seasonRating());
    newData.setAired(mediaTVSearchPtr->aired());
    newData.setNetworks(mediaTVSearchPtr->networks());
    newData.setSeason(mediaTVSearchPtr->season());
    newData.setEpisode(mediaTVSearchPtr->episode());
    newData.setEpisodeTitle(mediaTVSearchPtr->episodeTitle());
    c->clear();

    addImages(  scraper,  QStringList() << mediaTVSearchPtr->foundResult().getPosterHref(), QList<QSize>(), ImageType::Poster);

    addImages(  scraper,   mediaTVSearchPtr->postersHref(), mediaTVSearchPtr->postersSize(),ImageType::Poster);

    addImages(  scraper, mediaTVSearchPtr->backdropsHref(), mediaTVSearchPtr->backdropsSize(), ImageType::Backdrop);

    addImages(  scraper,  mediaTVSearchPtr->bannersHref(), mediaTVSearchPtr->bannersSize(),ImageType::Banner);

    addImages(  scraper, mediaTVSearchPtr->thumbailHref(), mediaTVSearchPtr->thumbailSize(),ImageType::Thumbnail);

    updateFrom(newData);
}


void PanelView::addImages( const Scraper* scraper, const QStringList&  hrefs, const QList<QSize>& sizes,  QFlags<ImageType> type){

    for (int i=0; i<hrefs.size(); i++){

        QString url= hrefs[i];
        if (!url.isEmpty()){
            QSize posterSize;
            if (sizes.size()>i){
                posterSize=sizes[i];
            }

            c->addImageFromScraper(scraper,url,posterSize,type);
        }
    }
}





void PanelView::foundMovie(const Scraper* scraper, MediaMovieSearchPtr mediaMovieSearchPtr){

  Q_UNUSED(scraper);

  CurrentItemData newData(mediaMovieSearchPtr->engine(),false, mediaMovieSearchPtr->mediaInfo(),mediaMovieSearchPtr->fileInfo());

  if (!ui->checkBoxLockSynopsis->isLock()){
      newData.setSynopsis(mediaMovieSearchPtr->synopsis());
  } else {
      newData.setSynopsis(ui->synopsis->toPlainText());
  }

  c->clear();

  addImages(  scraper,  QStringList() << mediaMovieSearchPtr->foundResult().getPosterHref(), QList<QSize>(), ImageType::Poster);

  addImages(  scraper,   mediaMovieSearchPtr->postersHref(), mediaMovieSearchPtr->postersSize(),ImageType::Poster);

  addImages(  scraper, mediaMovieSearchPtr->backdropsHref(), mediaMovieSearchPtr->backdropsSize(), ImageType::Backdrop);

  addImages(  scraper, mediaMovieSearchPtr->thumbailHref(), mediaMovieSearchPtr->thumbailSize(),ImageType::Thumbnail);

  updateFrom(newData);

#if 0
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
#endif
}


void  PanelView::enableSynopsis(bool enable){
    this->ui->synopsis->setReadOnly(!enable);

    if (!enable){
        //setSynopsis(this->ui->synopsis->toPlainText());
        //rebuildTemplate();
    }
}



void PanelView::setBanner(const QString& url, const QSize& originalSize, const Scraper *_currentScrape ){
    currentSearch._banner=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._banner.resources().isEmpty()){

        InProgressDialog* p=InProgressDialog::create();

        QString url=currentSearch._banner.scraper()->getBestImageUrl(currentSearch._banner.resources(),originalSize,currentSearch.engine()->getPosterSize(), Qt::KeepAspectRatio,ImageType::Banner);

        Promise* promise=Promise::loadAsync(manager,url,false);

        //  QObject::connect(this, &templateYadis::canceled, promise, &Promise::canceled);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray qb=promise->replyData();
                QPixmap bannerPixmap;
                if (bannerPixmap.loadFromData(qb)){
                    setBannerState(NETRESOURCE::OK, bannerPixmap);
                }else {
                    setBannerState(NETRESOURCE::ERROR);
                }
            } else if (promise->replyError() ==QNetworkReply::OperationCanceledError){
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

        QString url=currentSearch._thumbnail.scraper()->getBestImageUrl(currentSearch._thumbnail.resources(),originalSize,currentSearch.engine()->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false,QNetworkRequest::Priority::HighPriority);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray qb=promise->replyData();
                QPixmap thumbnail;
                if (thumbnail.loadFromData(qb)){
                    setThumbnailState(NETRESOURCE::OK, thumbnail);
                }else {
                    setThumbnailState(NETRESOURCE::ERROR);
                }
            } else if (promise->replyError() ==QNetworkReply::OperationCanceledError){
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

        QString url=currentSearch._poster.scraper()->getBestImageUrl(currentSearch._poster.resources(),originalSize,currentSearch.engine()->getPosterSize());

        Promise* promise=Promise::loadAsync(manager,url,false,QNetworkRequest::Priority::HighPriority);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray qb=promise->replyData();
                QPixmap fanArtPixmap;
                if (fanArtPixmap.loadFromData(qb)){
                    setPosterState(NETRESOURCE::OK, fanArtPixmap);
                }else {
                    setPosterState(NETRESOURCE::ERROR);
                }
            } else if (promise->replyError() ==QNetworkReply::OperationCanceledError){
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

bool setMediaState(NETRESOURCE mediaState, const QPixmap& media, NETRESOURCE& currentMediaState, QPixmap& currentMedia) {
    bool same=(mediaState == currentMediaState);
    if (same){
        same = media.toImage()==currentMedia.toImage();
    }
    if (!same){
        currentMedia        = media;
        currentMediaState   = mediaState;
        return true;
    }

    return false;

}

void PanelView::setBackdropState(NETRESOURCE backdropState, const QPixmap& backDrop){
    if (setMediaState(backdropState, backDrop,currentSearch.backdropState, currentSearch.currentBackdrop)){
        ui->chooseBackgroundButton->setMedia(backDrop);
        if (currentSearch.engine()) currentSearch.engine()->preview(currentSearch);
    }
}

void PanelView::setPosterState(NETRESOURCE posterState, const QPixmap& poster){
    if (setMediaState(posterState, poster,currentSearch.posterState, currentSearch.currentPoster)){
        ui->choosePosterButton->setMedia(poster);
        if (currentSearch.engine()) currentSearch.engine()->preview(currentSearch);
    }
}

void PanelView::setThumbnailState(NETRESOURCE thumbnailState, const QPixmap& thumbnail){
    if (setMediaState(thumbnailState, thumbnail,currentSearch.thumbnailState, currentSearch.currentThumbail)){
        ui->chooseThumbailButton->setMedia(thumbnail);
        if (currentSearch.engine()) currentSearch.engine()->preview(currentSearch);
    }
}

void PanelView::setBannerState(NETRESOURCE bannerState, const QPixmap& banner){

    if (setMediaState(bannerState, banner,currentSearch.bannerState, currentSearch.currentBanner)){
        ui->chooseBannerButton->setMedia(banner);
        if (currentSearch.engine()) currentSearch.engine()->preview(currentSearch);
    }
}
void PanelView::setBackdrop(const QString& url, const QSize& originalSize,const Scraper *_currentScrape){

    currentSearch._backdrop=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._backdrop.resources().isEmpty()){
        QString url=currentSearch._backdrop.scraper()->getBestImageUrl(currentSearch._backdrop.resources(),originalSize,currentSearch.engine()->getBackdropSize(),Qt::KeepAspectRatioByExpanding);

        InProgressDialog* p=InProgressDialog::create();

        Promise* promise=Promise::loadAsync(manager,url,false);

        //  QObject::connect(this, &templateYadis::canceled, promise, &Promise::canceled);

        QObject::connect(promise, &Promise::completed, [=]()
        {
            if (promise->replyError() ==QNetworkReply::NoError){
                QByteArray qb=promise->replyData();
                QPixmap fanArtPixmap;
                if (fanArtPixmap.loadFromData(qb)){
                    setBackdropState(NETRESOURCE::OK, fanArtPixmap);
                }else {
                    setBackdropState(NETRESOURCE::ERROR);
                }
            } else if (promise->replyError() ==QNetworkReply::OperationCanceledError){
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




void PanelView::previewOK(QGraphicsScene* s){
    ui->graphicsView->setScene(s);
}

void PanelView::proceed(){
    currentSearch.engine()->proceed(currentSearch);
}

void PanelView::rescrap() {
#if 0
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
#endif

}


void PanelView::backgroundSelected(const MediaChoosed& mediaChoosed){

    if (mediaChoosed.isEmpty()){
          setBackdropState(NETRESOURCE::NONE);
    } else if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        setBackdrop(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}

void PanelView::posterSelected(const MediaChoosed& mediaChoosed){
    if (mediaChoosed.isEmpty()){
          setPosterState(NETRESOURCE::NONE);
    } else if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        this->setPoster(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}

void PanelView::thumbnailSelected(const MediaChoosed& mediaChoosed){
    if (mediaChoosed.isEmpty()){
         setThumbnailState(NETRESOURCE::NONE);
    } else   if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        this->setThumbnail(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}

void PanelView::bannerSelected(const MediaChoosed& mediaChoosed){
    if (mediaChoosed.isEmpty()){
         setBannerState(NETRESOURCE::NONE);
    } else if (mediaChoosed.isMediaUrl()){
        qDebug() << mediaChoosed.url().toDisplayString();
    } else if (mediaChoosed.isMediaLocalFilePath()){
        qDebug() << mediaChoosed.localFilePath();

    } else if (mediaChoosed.isMediaScraper()){
        this->setBanner(mediaChoosed.scraperResource().resources(),mediaChoosed.scraperResource().originalSize(),mediaChoosed.scraperResource().scraper());
        qDebug() << mediaChoosed.scraperResource().resources();
    }
}
