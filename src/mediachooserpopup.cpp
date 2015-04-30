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

#include "mediachooserbutton.h"

Q_LOGGING_CATEGORY(mediaChooser, "ui.mediaChooser")

class MediaChooserGraphicsObject : public QGraphicsObject {

    QGraphicsPixmapItem* const m_p;
    QGraphicsItemGroup * m_group;
    const QFlags<ImageType> m_type;

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
        return false ; //m_p->sceneEvent(event);
    }

};

class ClickQGraphicsPixmapItem : public QGraphicsPixmapItem {
    QPointer<MediaChooserPopup> _itemToUpdate;
    QUrl _url;

public :

    ClickQGraphicsPixmapItem( QPointer<MediaChooserPopup> itemToUpdate,const QUrl url, const QPixmap &pixmap, QGraphicsItem *parent = 0)
        : QGraphicsPixmapItem (pixmap,parent),
          _itemToUpdate(itemToUpdate), _url(url){
        setFlag(QGraphicsItem::ItemIsSelectable);
    }

    enum { Type = UserType + 2 };

    int type() const
    {
        return Type;
    }


    void click(){
        if (!_itemToUpdate.isNull()){
            _itemToUpdate->mediaSelected(_url);
        }
    }
};

class GraphicsScene : public QGraphicsScene {
public:
    GraphicsScene(QObject *parent = 0) : QGraphicsScene(parent){}
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e) {
        QGraphicsItem *item= this->itemAt( e-> scenePos(),QTransform());
        if (item && item->type()==ClickQGraphicsPixmapItem::Type){
            ClickQGraphicsPixmapItem* m = static_cast<ClickQGraphicsPixmapItem*>(item);
            m->click();
        }
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e){}
};


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

void MediaChooserPopup::setScene(QGraphicsScene* scene){
    ui->graphicsView->setScene(scene);
}


void MediaChooserPopup::closeEvent(QCloseEvent *event)
{
       event->accept();
       emit popupClosed();
}

static QPixmap createDefaultPoster(int w, int h){
    QPixmap result(w,h);
    result.fill(Qt::white);

    /*  QPainter pixPaint(&result);

    QPixmap icon;
    icon.load(":/DownloadIcon.png");
    pixPaint.drawPixmap((w-icon.width())/2,(h-icon.height())/2,icon.width(),icon.height(),icon);
*/
    return result;
}

void MediaChooserPopup::setImageFromInternet(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate, int x, int y, int w, int h){

    if (itemToUpdate.isNull()){
        // Nothing to do
        return;
    }

    QPixmap scaled = (pixmap.width()>w || pixmap.height()>h) ? pixmap.scaled(w,h,Qt::KeepAspectRatio):pixmap;

    itemToUpdate->graphicsPixmapItem()->setPixmap(scaled);
    itemToUpdate->parentItem()->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);
}

void MediaChooserPopup::addFile(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator, int x, int y, int w, int h){

    if (!busyIndicator.isNull()){
        busyIndicator->setVisible(false);
    }

    QPixmap pixmap;
    if (pixmap.load(url.toLocalFile())){
        setImageFromInternet(pixmap,itemToUpdate, x, y, w, h);
    } else {
        addError(QString(tr("Unable to load file %1")).arg(url.toLocalFile()),itemToUpdate,busyIndicator, x, y, w, h);
    }
}

void MediaChooserPopup::addError(const QString& errorMessage,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator,  int x, int y, int w, int h){
    if (!busyIndicator.isNull()){
        busyIndicator->setVisible(false);
    }


    QPixmap result(w,h);
    result.fill(Qt::transparent);

    QPainter pixPaint(&result);
    pixPaint.drawText(x,y,w,h,Qt::AlignHCenter|Qt::TextWordWrap | Qt::AlignVCenter, errorMessage);

    setImageFromInternet(result,itemToUpdate, x, y, w, h);
}

void MediaChooserPopup::addImageFormUrl(const QUrl& url, QFlags<ImageType> type){
    QGraphicsScene *scene = ui->graphicsView->scene();

    int x=0;
    int y=20;
    int w=200;
    int h=200;

    QList<QGraphicsItem*> groupItems;

    groupItems.append(scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern)));

    QPixmap scaled = createDefaultPoster(w,h);

    MediaChooserGraphicsObject* pi=new MediaChooserGraphicsObject(scene->addPixmap(scaled), type);
    pi->parentItem()->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);
    groupItems.append(pi->parentItem());

    QLabel *gif_anim = new QLabel();
    QMovie *movie = new QMovie(":/ressources/animations/busy-1.gif");
    gif_anim->setMovie(movie);
    movie->start();

    QGraphicsProxyWidget *proxy = scene->addWidget(gif_anim);
    proxy->setPos(x+(w-32)/2,y+(h-32)/2);
    groupItems.append(proxy);

    ClickQGraphicsPixmapItem *pbutton= new ClickQGraphicsPixmapItem(QPointer<MediaChooserPopup>(this), url, QPixmap(":/ressources/check.png"));
    scene->addItem(pbutton);
    pbutton->setPos(x,h);
    groupItems.append(pbutton);

    QGraphicsItemGroup * cliGroup = scene->createItemGroup(groupItems);
    cliGroup->setPos(0,y);

    pi->setItemGroup(cliGroup);

    if (!url.isValid()){
        addError(QString(tr("Invalid URL: %1")).arg(url.toString()),QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxy), x,  y,  w,  h);
    } else if (url.scheme()=="file"){
        addFile(url,QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxy), x,  y,  w,  h);
    } else if (url.scheme()=="http"){
    } else {
        addError(QString(tr("Don't known how to open %1")).arg(url.toDisplayString()),QPointer<MediaChooserGraphicsObject>(pi), QPointer<QGraphicsProxyWidget>(proxy), x,  y,  w,  h);
    }
}


void MediaChooserPopup::addImageFromFile(const QString& localFile, QFlags<ImageType> type){
    addImageFormUrl(QUrl::fromLocalFile(localFile),type);
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
    foreach (QGraphicsItem *item, ui->graphicsView->scene()->items(Qt::AscendingOrder))
    {
        if (item->type()==MediaChooserGraphicsObject::Type){
            MediaChooserGraphicsObject* m = static_cast<MediaChooserGraphicsObject*>(item);
            if (m->group()){

                if (m->imageType() & filter){
                    m->group()->setVisible(true);
                    m->group()->setHandlesChildEvents(false);
                    m->group()->setX(x);

                    x+=210;
                } else {
                    m->group()->setVisible(false);
                  }
            }
        }
    }


//ui->graphicsView->setSceneRect(QRect());

//ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);

    const QSignalBlocker blocker0(ui->checkBoxThumbail);
    const QSignalBlocker blocker1(ui->checkBoxBackdrop);
    const QSignalBlocker blocker2(ui->checkBoxBanner);
    const QSignalBlocker blocker3(ui->checkBoxPoster);
    ui->checkBoxThumbail->setChecked(filter & ImageType::Thumbnail);
    ui->checkBoxBackdrop->setChecked(filter & ImageType::Backdrop);
    ui->checkBoxBanner->setChecked(filter & ImageType::Banner);
    ui->checkBoxPoster->setChecked(filter & ImageType::Poster);
}

QFlags<ImageType> MediaChooserPopup::currentFilter(){
    return _currentFilter;
}

void MediaChooserPopup::popup(MediaChooserButton *button, QFlags<ImageType> filter){

    disconnect(SIGNAL(popupClosed()));
    connect(this, SIGNAL(popupClosed()), button, SLOT(popupClosed()), Qt::UniqueConnection);

    disconnect(SIGNAL(mediaSelected(const QUrl& )));
    connect(this,SIGNAL(mediaSelected(const QUrl&)), button,SLOT(mediaSelected(const QUrl&)), Qt::UniqueConnection);

    doFilter(filter);

    show();
}




void MediaChooserPopup::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

}
