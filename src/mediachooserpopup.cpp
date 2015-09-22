#include "mediachooserpopup.h"
#include "ui_mediachooserpopup.h"

#include <QGraphicsScene>
#include <QCloseEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QMovie>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QFileDialog>
#include <QDebug>
#include <QNetworkAccessManager>

#include "mediachooserbutton.h"
#include "scrapers/scraper.h"
#include "promise.h"
#include "blocker.h"

Q_LOGGING_CATEGORY(mediaChooser, "ui.mediaChooser")

class MediaChooserGraphicsObject : public QGraphicsObject {

    QGraphicsPixmapItem* const m_p;
    QGraphicsItemGroup * m_group;
    const QFlags<ImageType> m_type;
    QList<QGraphicsItem*> _groupItems;

public :
    MediaChooserGraphicsObject(QGraphicsPixmapItem* p,  QFlags<ImageType> type) : QGraphicsObject(p), m_p(p), m_group(nullptr), m_type(type){

    }

    QRectF boundingRect() const
    {
        return m_p->boundingRect();
    }

    QPainterPath shape() const
    {
        return m_p->shape();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        m_p->paint(painter,  option, widget);
    }


    QGraphicsPixmapItem* graphicsPixmapItem(){
        return m_p;

    }

    void setItemGroup(QGraphicsItemGroup * group){
        m_group=group;
    }

    QGraphicsItemGroup* group(){
        return m_group;
    }

    QFlags<ImageType> imageType(){
        return m_type;
    }

    enum { Type = UserType + 1 };

    int type() const
    {
        return Type;
    }

protected:
    bool sceneEvent(QEvent *event){
        Q_UNUSED(event);
        return false ; //m_p->sceneEvent(event);
    }

};

class ClickButtonMediaChooserGraphicsObject : public QGraphicsObject {

    QGraphicsPixmapItem* const m_p;
    QPointer<MediaChooserPopup> _itemToUpdate;
    MediaChoosed _mediaChoosed;

public :
    ClickButtonMediaChooserGraphicsObject(QGraphicsPixmapItem* p, QPointer<MediaChooserPopup> itemToUpdate,const MediaChoosed& mediaChoosed) :
        QGraphicsObject(p),
        m_p(p),
        _itemToUpdate(itemToUpdate),
        _mediaChoosed(mediaChoosed)
    {
        setFlag(QGraphicsItem::ItemIsSelectable);
    }

    QRectF boundingRect() const
    {
        return m_p->boundingRect();
    }

    QPainterPath shape() const
    {
        return m_p->shape();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        m_p->paint(painter,  option, widget);
    }

    enum { Type = UserType + 3 };

    int type() const
    {
        return Type;
    }


    virtual void click(){
        if (!_itemToUpdate.isNull()){
            _itemToUpdate->mediaSelected(_mediaChoosed);
        }
    }

protected:
    bool sceneEvent(QEvent *event){
        Q_UNUSED(event);
        return false ; //m_p->sceneEvent(event);
    }

};

class ClickButtonMediaChooserGraphicsObject2 : public ClickButtonMediaChooserGraphicsObject {
    QUrl _url;
    QPointer<MediaChooserPopup> _itemToUpdate;
    QPointer<QGraphicsProxyWidget> _busyIndicator;
    QPointer<ClickButtonMediaChooserGraphicsObject> _chooseButton;
    QPointer<MediaChooserGraphicsObject> _itemToUpdate2;

public :
    ClickButtonMediaChooserGraphicsObject2(QGraphicsPixmapItem* p, QPointer<MediaChooserPopup> itemToUpdate,const MediaChoosed& mediaChoosed, const QUrl& url, QPointer<MediaChooserGraphicsObject> itemToUpdate2,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton) :
        ClickButtonMediaChooserGraphicsObject(p,itemToUpdate,mediaChoosed){
        _url=url;
        _itemToUpdate=itemToUpdate;
        _busyIndicator=busyIndicator;
        _chooseButton=chooseButton;
        _itemToUpdate2=itemToUpdate2;
    }

    virtual void click(){
        if (!_itemToUpdate.isNull()){
            _itemToUpdate->reload(_url,_itemToUpdate2,_busyIndicator,_chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject>(this));
        }

    }
};

class GraphicsScene : public QGraphicsScene {
public:
    GraphicsScene(QObject *parent = 0) : QGraphicsScene(parent){}
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e) {
        QGraphicsItem *item= this->itemAt( e-> scenePos(),QTransform());
        if (item && item->type()==ClickButtonMediaChooserGraphicsObject::Type){
            ClickButtonMediaChooserGraphicsObject* m = static_cast<ClickButtonMediaChooserGraphicsObject*>(item);
            m->click();
        }
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *){}
};


QNetworkAccessManager manager;

struct M_M {
    QString url;
    QPointer<MediaChooserGraphicsObject> itemToUpdate;
    QPointer<QGraphicsProxyWidget> busyIndicator;
    QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton;
    QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton;

};

QList<M_M> urls;
QPointer<Promise> currentPromise;

void MediaChooserPopup::startPromise( QNetworkAccessManager* manager){

    if (urls.isEmpty() || !currentPromise.isNull()){
        return;
    }

    M_M url=urls.takeFirst();

    if (!url.busyIndicator.isNull()){
        url.busyIndicator->setVisible(true);
    }

    if (!url.chooseButton.isNull()){
        url.chooseButton->parentItem()->hide();
    }

    if (!url.reloadButton.isNull()){
        url.reloadButton->parentItem()->hide();
    }

    currentPromise=Promise::loadAsync(*manager,url.url,true,true,QNetworkRequest::NormalPriority);

    QObject::connect(currentPromise, &Promise::completed, [=]()
    {

        if (currentPromise->replyError() ==QNetworkReply::NoError){
            QByteArray qb=currentPromise->replyData();
            QPixmap px;
            if (px.loadFromData(qb)){
                addPixmap(px,url.itemToUpdate,url.busyIndicator,url.chooseButton,url.reloadButton);
            } else {
                addError(QString(tr("Unable to load %1")).arg(url.url),url.itemToUpdate,url.busyIndicator,url.chooseButton, url.reloadButton);
            }

        } else {
            addError(currentPromise->replyErrorString(),url.itemToUpdate,url.busyIndicator,url.chooseButton,url.reloadButton);

        }

        currentPromise.clear();

        startPromise(manager);
    });

}

MediaChooserPopup::MediaChooserPopup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaChooserPopup)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    ui->setupUi(this);

    ui->graphicsView->setScene(new GraphicsScene(this));

}

MediaChooserPopup::~MediaChooserPopup()
{
    delete ui;
}

void  MediaChooserPopup::reload(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton){
    qDebug() << "reload " << url;
    addHttpRequest(url,itemToUpdate,   busyIndicator,chooseButton,  reloadButton);
}


void MediaChooserPopup::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit popupClosed();
}

QPixmap MediaChooserPopup::createDefaultPoster() const{
    QPixmap result(getMediaWidth(),getMediaHeigth());
    result.fill(Qt::white);
    return result;
}

void MediaChooserPopup::addPixmap(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate,QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton){
    if (!busyIndicator.isNull()){
        busyIndicator->setVisible(false);
    }

    if (!chooseButton.isNull()){
        chooseButton->parentItem()->show();
    }

    if (!reloadButton.isNull()){
        reloadButton->parentItem()->hide();
    }


    setImageFromInternet(pixmap,itemToUpdate);
}

void MediaChooserPopup::setImageFromInternet(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate){

    if (itemToUpdate.isNull()){
        // Nothing to do
        return;
    }

    QPixmap scaled = (pixmap.width()>getMediaWidth() || pixmap.height()>getMediaHeigth()) ? pixmap.scaled(getMediaWidth(),getMediaHeigth(),Qt::KeepAspectRatio,Qt::SmoothTransformation):pixmap;

    itemToUpdate->graphicsPixmapItem()->setPixmap(scaled);
    itemToUpdate->parentItem()->setPos(0+(getMediaWidth()-scaled.width())/2,0+(getMediaHeigth()-scaled.height())/2);
}

void MediaChooserPopup::addFile(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator,QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton){

    QPixmap pixmap;
    if (pixmap.load(url.toLocalFile())){
        addPixmap(pixmap,itemToUpdate,busyIndicator,chooseButton,reloadButton);
    } else {
        addError(QString(tr("Unable to load file %1")).arg(url.toLocalFile()),itemToUpdate,busyIndicator, chooseButton,reloadButton);
    }
}

void MediaChooserPopup::addError(const QString& errorMessage,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator,  QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton){
    if (!busyIndicator.isNull()){
        busyIndicator->setVisible(false);
    }

    if (!chooseButton.isNull()){
        chooseButton->parentItem()->hide();
    }

    if (!reloadButton.isNull()){
        reloadButton->parentItem()->show();
    }
    QPixmap result(getMediaWidth(),getMediaHeigth());
    result.fill(Qt::transparent);

    QPainter pixPaint(&result);
    pixPaint.drawText(0,0,getMediaWidth(),getMediaHeigth(),Qt::AlignHCenter|Qt::TextWordWrap | Qt::AlignVCenter, errorMessage);

    setImageFromInternet(result,itemToUpdate);
}

void MediaChooserPopup::addImageFromScraper( const Scraper* scraper, const QString& imageRef, const QSize& originalSize, QFlags<ImageType> type ){
    QLoggingCategory fcIo("ui.mediaChooser");

    QString realUrl=scraper->getBestImageUrl(imageRef,originalSize,QSize(200,200),Qt::KeepAspectRatio,type);

    qCDebug(fcIo) << "addImageFromScraper " << imageRef << realUrl;

    addImage(QUrl(realUrl),MediaChoosed(ScraperResource(imageRef,originalSize, scraper)), type);
}

void MediaChooserPopup::addImageFromUrl(const QUrl& url, QFlags<ImageType> type){
    addImage(url,MediaChoosed(url),type);
}

void MediaChooserPopup::addImage(const QUrl& url, const MediaChoosed& mediaChoosed, QFlags<ImageType> type){

    QGraphicsScene *scene = ui->graphicsView->scene();


    QList<QGraphicsItem*> groupItems;

    groupItems.append(scene->addRect(0,0,getMediaWidth(),getMediaHeigth(), QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern)));

    QPixmap scaled = createDefaultPoster();

    MediaChooserGraphicsObject* pi=new MediaChooserGraphicsObject(scene->addPixmap(scaled), type);
    pi->parentItem()->setPos(0+(getMediaWidth()-scaled.width())/2,0+(getMediaHeigth()-scaled.height())/2);
    groupItems.append(pi->parentItem());

    QLabel *gif_anim = new QLabel();
    QMovie *movie = new QMovie(":/resources/animations/busy-1.gif");
    gif_anim->setMovie(movie);
    movie->start();

    QGraphicsProxyWidget *proxyGifAnim = scene->addWidget(gif_anim);
    proxyGifAnim->setPos(0+(getMediaWidth()-32)/2,0+(getMediaHeigth()-32)/2);
    groupItems.append(proxyGifAnim);

    QGraphicsPixmapItem* bt = scene->addPixmap(QPixmap(":/resources/images/bingo16x16.png"));
    ClickButtonMediaChooserGraphicsObject* vv = new ClickButtonMediaChooserGraphicsObject(bt, QPointer<MediaChooserPopup>(this), mediaChoosed);
    vv->parentItem()->setPos(0,getMediaHeigth());
    groupItems.append(vv->parentItem());
    vv->parentItem()->hide();

    QGraphicsPixmapItem* bt2 = scene->addPixmap(QPixmap(":/resources/images/reload.png"));
    ClickButtonMediaChooserGraphicsObject2* vv2 = new ClickButtonMediaChooserGraphicsObject2(bt2, QPointer<MediaChooserPopup>(this), mediaChoosed, url, QPointer<MediaChooserGraphicsObject>(pi),  QPointer<QGraphicsProxyWidget>(proxyGifAnim), QPointer<ClickButtonMediaChooserGraphicsObject>(vv));
    vv2->parentItem()->setPos(0,getMediaHeigth());
    groupItems.append(vv2->parentItem());
    vv2->parentItem()->hide();

    QGraphicsItemGroup * cliGroup = scene->createItemGroup(groupItems);
    cliGroup->setPos(0,0);

    pi->setItemGroup(cliGroup);

    if (!url.isValid()){
        addError(QString(tr("Invalid URL: %1")).arg(url.toString()),QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxyGifAnim), QPointer<ClickButtonMediaChooserGraphicsObject>(vv), QPointer<ClickButtonMediaChooserGraphicsObject>(vv2));
    } else if (url.scheme()=="file"){
        addFile(url,QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxyGifAnim),QPointer<ClickButtonMediaChooserGraphicsObject>(vv),QPointer<ClickButtonMediaChooserGraphicsObject>(vv2));
    } else if (url.scheme()=="http"){
        addHttpRequest(url, QPointer<MediaChooserGraphicsObject>(pi),  QPointer<QGraphicsProxyWidget>(proxyGifAnim), QPointer<ClickButtonMediaChooserGraphicsObject>(vv), QPointer<ClickButtonMediaChooserGraphicsObject>(vv2) );
    } else {
        addError(QString(tr("Don't known how to open %1")).arg(url.toDisplayString()),QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxyGifAnim), QPointer<ClickButtonMediaChooserGraphicsObject>(vv),QPointer<ClickButtonMediaChooserGraphicsObject>(vv2));
    }
}


void MediaChooserPopup::addHttpRequest(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton){
    M_M f;

    f.url=url.toString();

    f.itemToUpdate=itemToUpdate;
    f.busyIndicator=busyIndicator;
    f.chooseButton=chooseButton;
    f.reloadButton= reloadButton;

    urls.append(f);

    startPromise(&manager);
}


void MediaChooserPopup::addImageFromFile(const QString& localFile, QFlags<ImageType> type){
    addImage(QUrl::fromLocalFile(localFile),MediaChoosed(localFile), type);
}

void MediaChooserPopup::refilter(){

    QLoggingCategory fcIo("ui.mediaChooser");
    qCDebug(fcIo) << "refilter";

    QFlags<ImageType> filter=_currentFilter;

    if (ui->checkBoxBanner->isChecked()){
        filter |=ImageType::Banner;
    } else {
        filter &= (~QFlags<ImageType>(ImageType::Banner));
    }

    if (ui->checkBoxBackdrop->isChecked()){
        filter |=ImageType::Backdrop;
    } else {
        filter &= (~QFlags<ImageType>(ImageType::Backdrop));
    }

    if (ui->checkBoxPoster->isChecked()){
        filter |=ImageType::Poster;
    } else {
        filter &= (~QFlags<ImageType>(ImageType::Poster));
    }

    if (ui->checkBoxThumbail->isChecked()){
        filter |=ImageType::Thumbnail;
    } else {
        filter &= (~QFlags<ImageType>(ImageType::Thumbnail));
    }

    qCDebug(fcIo) << filter;

    doFilter(filter);
}

void MediaChooserPopup::doFilter( QFlags<ImageType> filter){

    _currentFilter=filter;

    int x=0;
    int w=0;
    foreach (QGraphicsItem *item, ui->graphicsView->scene()->items(Qt::AscendingOrder))
    {
        if (item->type()==MediaChooserGraphicsObject::Type){
            MediaChooserGraphicsObject* m = static_cast<MediaChooserGraphicsObject*>(item);
            if (m->group()){



                if (m->imageType() & filter){
                    m->group()->show();

                    m->group()->setHandlesChildEvents(false);
                    m->group()->setX(x);

                    x+=210;
                    w+=210;
                } else {
                    m->group()->hide();
                    m->group()->setX(0); // for itemsBoundingRect
                }
            }
        }
    }


    whileBlocking(ui->checkBoxThumbail)->setChecked(filter & ImageType::Thumbnail);
    whileBlocking(ui->checkBoxBackdrop)->setChecked(filter & ImageType::Backdrop);
    whileBlocking(ui->checkBoxBanner)->setChecked(filter & ImageType::Banner);
    whileBlocking(ui->checkBoxPoster)->setChecked(filter & ImageType::Poster);

    ui->graphicsView->setSceneRect(ui->graphicsView->scene()->itemsBoundingRect());
}

QFlags<ImageType> MediaChooserPopup::currentFilter(){
    return _currentFilter;
}

void MediaChooserPopup::popup(MediaChooserButton *button, QFlags<ImageType> filter){

    disconnect(SIGNAL(popupClosed()));
    connect(this, SIGNAL(popupClosed()), button, SLOT(popupClosed()), Qt::UniqueConnection);

    disconnect(SIGNAL(mediaChoosed(const MediaChoosed& )));
    connect(this,SIGNAL(mediaChoosed(const MediaChoosed&)), button,SLOT(mediaSelected(const MediaChoosed&)), Qt::UniqueConnection);

    doFilter(filter);

    show();
}

void MediaChooserPopup::clear(){
    urls.clear();

    ui->graphicsView->scene()->clear();



}

void MediaChooserPopup::mediaSelected(const MediaChoosed& url){
    qDebug() << url.localFilePath();
    emit mediaChoosed(url);
}



void MediaChooserPopup::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

}

