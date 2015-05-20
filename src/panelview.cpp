#include "panelview.h"
#include "ui_panelview.h"

#include <QDebug>
#include <QNetworkReply>
#include <QGraphicsProxyWidget>
#include <QGraphicsObject>
#include <QPushButton>
#include <QMovie>
#include <QCompleter>
#include <QDir>

#include <limits>

#include "scrapers/themoviedbscraper.h"
#include "scrapers/allocinescraper.h"
#include "scrapers/thetvdbscraper.h"
#include "scrapers/defaultscraper.h"
#include "searchscraperdialog.h"
#include "promise.h"
#include "scanner/mediainfoscanner.h"
#include "engine/fileengine.h"
#include "mediachooserpopup.h"
#include "blocker.h"
#include "jaqetmainwindow.h"


MediaChooserPopup* c;

class FlagWidgetItem: public QListWidgetItem {
public:


    FlagWidgetItem(const QString &text, QListWidget *view = 0, int type = Type):
    QListWidgetItem(text,view,type){

    }

    bool operator<(const QListWidgetItem &other) const{
        int d1=data(Qt::UserRole).toInt();
        int d2=other.data(Qt::UserRole).toInt();

        if (d1<d2){
            return true;
        } else if (d1==d2){
            return QString::compare(this->text(),other.text())<0;

        }

        return false;
    }

};

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

    whileBlocking(ui->countriesListWidget, [&](){
        QDir flags(":/resources/images/flags");
        QRegularExpression flagNamePattern("flag_([a-zA-Z][a-zA-Z]).png");
        for (const QString& flag : flags.entryList()){
            QRegularExpressionMatch match = flagNamePattern.match(flag);
            if (match.hasMatch()) {
                FlagWidgetItem* item = new FlagWidgetItem( match.captured(1), ui->countriesListWidget);
                 item->setIcon(QIcon(QFileInfo(flags,flag).absoluteFilePath()));
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);
                item->setData(Qt::UserRole,std::numeric_limits<int>::max());
            }
        }
    });

    ui->countriesListWidget->setSortingEnabled(true);

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

    FileEngine* fileEngine = static_cast<FileEngine*>(engine);
    ui->graphicsViewBingo->setScene(&fileEngine->preview(index));
    ui->graphicsViewPosterBingo->setScene(&fileEngine->poster(index));

}

void PanelView::setDir(){
    ui->stackedWidget->setCurrentIndex(2);
}

void PanelView::reset(){
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

                    fd.getResult().getScraper()->findMovieInfo(&this->manager,MediaMovieSearchPtr(new MediaMovieSearch(fileInfo,r.mediaInfo,fd.getResult(),engine)), Scraper::SearchOption::All,fd.getResult().language());
                } else {
                    fd.getResult().getScraper()->findEpisodeInfo(&this->manager,MediaTVSearchPtr(new MediaTVSearch(fileInfo,r.mediaInfo,fd.getResult(),engine)), Scraper::SearchOption::All,fd.getResult().language());
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
    ui->originalTitleEdit->setText(currentSearch.originalTitle());
    ui->chooseBackgroundButton->setMedia(currentSearch.currentBackdrop);
    ui->choosePosterButton->setMedia(currentSearch.currentPoster);

    if (currentSearch.isTV()){
        ui->chooseBannerButton->setMedia(currentSearch.currentBanner);
        ui->panelBanner->setVisible(true);
    } else {
        ui->panelBanner->setVisible(false);
    }

    ui->chooseThumbailButton->setMedia(currentSearch.currentThumbail);


    whileBlocking(ui->countriesListWidget, [&](){
     for (int i = 0; i <  ui->countriesListWidget->count(); i++) {
            QListWidgetItem *item = ui->countriesListWidget->item(i);
           // qDebug() << item->text();
            int index=currentSearch.countries().indexOf(item->text());
            if (index>=0){
                item->setCheckState(Qt::Checked);
                item->setData(Qt::UserRole, index);
            } else {
                item->setCheckState(Qt::Unchecked);
                item->setData(Qt::UserRole, std::numeric_limits<int>::max());
            }
        }
     });


    if (!currentSearch.isTV()){
        ui->doubleSpinBoxRating->setValue(currentSearch.rating());
    }

    buildPreview(currentSearch);
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


    c->clear();

    addImages(  scraper,  QStringList() << mediaTVSearchPtr->foundResult().getPosterHref(), QList<QSize>(), ImageType::Poster);

    addImages(  scraper,   mediaTVSearchPtr->postersHref(), mediaTVSearchPtr->postersSize(),ImageType::Poster);

    addImages(  scraper, mediaTVSearchPtr->backdropsHref(), mediaTVSearchPtr->backdropsSize(), ImageType::Backdrop);

    addImages(  scraper,  mediaTVSearchPtr->bannersHref(), mediaTVSearchPtr->bannersSize(),ImageType::Banner);

    addImages(  scraper, mediaTVSearchPtr->thumbailHref(), mediaTVSearchPtr->thumbailSize(),ImageType::Thumbnail);

    updateFrom(newData);
}

void PanelView::foundMovie(const Scraper* scraper, MediaMovieSearchPtr mediaMovieSearchPtr){

    Q_UNUSED(scraper);

    CurrentItemData newData(mediaMovieSearchPtr->engine(),false, mediaMovieSearchPtr->mediaInfo(),mediaMovieSearchPtr->fileInfo());

    if (!ui->checkBoxLockSynopsis->isLock()){
        newData.setSynopsis(mediaMovieSearchPtr->synopsis());
    } else {
        newData.setSynopsis(ui->synopsis->toPlainText());
    }

    if (!ui->checkBoxLockCast->isLock()){
        newData.setActors(mediaMovieSearchPtr->actors());
    } else {
        newData.setActors( getItemsListWidget(ui->castListWidget));
    }

    if (!ui->checkBoxLockDirectors->isLock()){
        newData.setDirectors(mediaMovieSearchPtr->directors());
    } else {
        newData.setDirectors( getItemsListWidget(ui->directorListWidget));
    }

    if (!ui->checkBoxLockGenre->isLock()){
        newData.setGenre(mediaMovieSearchPtr->genre());
    } else {
        newData.setGenre(getItemsListWidget(ui->genreListWidget));
    }

    newData.setTitle(mediaMovieSearchPtr->title());
    newData.setOriginalTitle(mediaMovieSearchPtr->originalTitle());
    newData.setRuntimeInSec(mediaMovieSearchPtr->runtimeInSec());
    newData.setCountries(mediaMovieSearchPtr->countries());
    newData.setRating(mediaMovieSearchPtr->rating());

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


    c->clear();

    addImages(  scraper,  QStringList() << mediaMovieSearchPtr->foundResult().getPosterHref(), QList<QSize>(), ImageType::Poster);

    addImages(  scraper,   mediaMovieSearchPtr->postersHref(), mediaMovieSearchPtr->postersSize(),ImageType::Poster);

    addImages(  scraper, mediaMovieSearchPtr->backdropsHref(), mediaMovieSearchPtr->backdropsSize(), ImageType::Backdrop);

    addImages(  scraper, mediaMovieSearchPtr->thumbailHref(), mediaMovieSearchPtr->thumbailSize(),ImageType::Thumbnail);

    updateFrom(newData);

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

        JaqetMainWindow::getInstance()->showWaitDialog();

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

            JaqetMainWindow::getInstance()->hideWaitDialog();

        });

    } else {
        setPosterState(NETRESOURCE::NONE);
    }

}

void PanelView::setThumbnail (const QString& url, const QSize& originalSize, const Scraper *_currentScrape){

    currentSearch._thumbnail=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._thumbnail.resources().isEmpty()){

        JaqetMainWindow::getInstance()->showWaitDialog();

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

            JaqetMainWindow::getInstance()->hideWaitDialog();
        });

    } else {
        setThumbnailState(NETRESOURCE::NONE);
    }

}


void PanelView::setPoster (const QString& url, const QSize& originalSize, const Scraper *_currentScrape){

    currentSearch._poster=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._poster.resources().isEmpty()){

        JaqetMainWindow::getInstance()->showWaitDialog();

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

            JaqetMainWindow::getInstance()->hideWaitDialog();
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
        buildPreview(currentSearch);
    }
}

void PanelView::setPosterState(NETRESOURCE posterState, const QPixmap& poster){
    if (setMediaState(posterState, poster,currentSearch.posterState, currentSearch.currentPoster)){
        ui->choosePosterButton->setMedia(poster);
        buildPreview(currentSearch);
        buildPoster(currentSearch);
    }
}

void PanelView::setThumbnailState(NETRESOURCE thumbnailState, const QPixmap& thumbnail){
    if (setMediaState(thumbnailState, thumbnail,currentSearch.thumbnailState, currentSearch.currentThumbail)){
        ui->chooseThumbailButton->setMedia(thumbnail);
        buildPreview(currentSearch);
    }
}

void PanelView::setBannerState(NETRESOURCE bannerState, const QPixmap& banner){

    if (setMediaState(bannerState, banner,currentSearch.bannerState, currentSearch.currentBanner)){
        ui->chooseBannerButton->setMedia(banner);
        buildPreview(currentSearch);
    }
}


void PanelView::setBackdrop(const QString& url, const QSize& originalSize,const Scraper *_currentScrape){

    currentSearch._backdrop=ScraperResource(url,originalSize,_currentScrape);

    if (!currentSearch._backdrop.resources().isEmpty()){
        QString url=currentSearch._backdrop.scraper()->getBestImageUrl(currentSearch._backdrop.resources(),originalSize,currentSearch.engine()->getBackdropSize(),Qt::KeepAspectRatioByExpanding);

        Promise* promise=Promise::loadAsync(manager,url,false);

        JaqetMainWindow::getInstance()->showWaitDialog();

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

            JaqetMainWindow::getInstance()->hideWaitDialog();
      });

    } else {
        setBackdropState(NETRESOURCE::NONE);
    }

}

void PanelView::buildPoster( const CurrentItemData& currentSearch){
    if (!currentSearch.engine()){
        return;
    }

    ui->graphicsViewPoster->setScene(&currentSearch.engine()->poster(currentSearch));
}

void PanelView::buildPreview( const CurrentItemData& currentSearch){
    if (!currentSearch.engine()){
        return;
    }

    ui->graphicsView->setScene(&currentSearch.engine()->preview(currentSearch));
}


void PanelView::proceed(){
    currentSearch.engine()->proceed(currentSearch);
}

void PanelView::rescrap() {

    SearchScraperDialog fd(this, currentSearch, this->scrapes, &this->manager);
    if (fd.exec()==QDialog::Accepted){
        if (!fd.getResult().isNull()){
            if (!fd.getResult().isTV()){

                fd.getResult().getScraper()->findMovieInfo(&this->manager,MediaMovieSearchPtr(new MediaMovieSearch(currentSearch.fileInfo(),currentSearch.mediaInfo(),fd.getResult(),currentSearch.engine())), Scraper::SearchOption::All,fd.getResult().language());
            } else {
                fd.getResult().getScraper()->findEpisodeInfo(&this->manager,MediaTVSearchPtr(new MediaTVSearch(currentSearch.fileInfo(),currentSearch.mediaInfo(),fd.getResult(),currentSearch.engine())), Scraper::SearchOption::All,fd.getResult().language());
            }
        }
    }

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

void PanelView::on_countriesListWidget_itemChanged(QListWidgetItem *item)
{
    QStringList countries;
    QMap <int,QStringList> indexCountry;

    for (int i = 0; i <  ui->countriesListWidget->count(); i++) {
        QListWidgetItem *item = ui->countriesListWidget->item(i);
        if (item->checkState()==Qt::Checked){
            indexCountry[item->data(Qt::UserRole).toInt()]<<item->text();
        }
    }

    for (int i : indexCountry.keys()){
        countries << indexCountry[i];
    }

    currentSearch.setCountries(countries);

    updateUI();

   buildPreview(currentSearch);
}
