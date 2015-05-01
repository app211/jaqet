#include "mediachooserbutton.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QDebug>

#include "mediachooserpopup.h"

MediaChooserButton::MediaChooserButton(QWidget *parent) :
    QAbstractButton(parent),
    m_State  (STATE_NORMAL),
    m_erase(QPixmap(":/resources/images/erase.png")),
    m_indicator(QPixmap(":/resources/images/menu_indicator.png"))

{
    setContentsMargins(3,3,3,3);
    setMediaSize(QSize(50,50));
}

void MediaChooserButton::initStyleOption(QStyleOptionButton &option) const
{
    option.initFrom(this);
    option.features = QStyleOptionButton::None;

    if (isDown()){
        option.state |= QStyle::State_Sunken;
    } else {
        option.state |= QStyle::State_Raised;
    }

}


void MediaChooserButton::enterEvent(QEvent *)
{ 
    m_State = STATE_HOVERED;

    update();
}

void MediaChooserButton::leaveEvent(QEvent *)
{
    m_State = STATE_NORMAL;

    update();
}
void MediaChooserButton::focusOutEvent(QFocusEvent *e)
{
    if (_popup && _popup->isVisible()) {
        QWidget::focusOutEvent(e);
    } else {
        QAbstractButton::focusOutEvent(e);
    }
}



void MediaChooserButton::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QStyleOptionButton option;
    initStyleOption(option);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter,
                         this);

    int offy= this->m_erase.height()/2;
    QRect destinationRect= contentsRect ();
    QSize s=getTSize();
    int destiation_x = destinationRect.x()+(destinationRect.width()-s.width())/2;
    int destiation_y = destinationRect.y()+(destinationRect.height()-s.height())/2;

    if (!_mediaScaled.isNull()){
        painter.drawPixmap(destiation_x+(_mediaSize.width()-_mediaScaled.width())/2, destiation_y+offy+( _mediaSize.height()-_mediaScaled.height())/2, _mediaScaled.width(), _mediaScaled.height(),_mediaScaled);

        if (m_State==STATE_HOVERED){
            painter.drawPixmap(eraseIconRect(),m_erase);
        }
    } else {
        // if (m_State==STATE_HOVERED){

        //  painter.setBrush(background);
        painter.setPen  (QPen(QColor(0,0,0)) ); // No stroke

        painter.drawRect(destiation_x,destiation_y+ offy, _mediaSize.width()-1, _mediaSize.height()-1);
    }

    // Menu indicator
    painter.drawPixmap(destiation_x+(_mediaSize.width()-m_indicator.width())/2, destiation_y+offy+ _mediaSize.height(), m_indicator.width(), m_indicator.height(),m_indicator);

}

QRect MediaChooserButton::eraseIconRect() const{
    if (!_mediaScaled.isNull()){
        QRect destinationRect= contentsRect ();
        QSize s=getTSize();
        int destiation_x = destinationRect.x()+(destinationRect.width()-s.width())/2;
        int destiation_y = destinationRect.y()+(destinationRect.height()-s.height())/2;

        return QRect(destiation_x+(_mediaSize.width()-_mediaScaled.width())/2+_mediaScaled.width()-m_erase.width()/2, destiation_y+( _mediaSize.height()-_mediaScaled.height())/2, m_erase.width(), m_erase.height());
    }

    return QRect();
}

QSize MediaChooserButton::getTSize() const {
    return QSize(qMax<int>(_mediaSize.width()+m_indicator.width()/2,m_indicator.width()),_mediaSize.height()+m_indicator.height()+m_erase.height()/2);
}

QSize MediaChooserButton::sizeHint() const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    return getTSize() + QSize(left+right, top+bottom);
}

bool MediaChooserButton::hitButton(const QPoint &pos) const {
    QRect r=eraseIconRect();

    if (r.isNull() || !r.contains(pos)){
        return true;
    }

    return false;
}

bool MediaChooserButton::hitEraseButton(const QPoint &pos) const {
    QRect r=eraseIconRect();

    if (!r.isNull() && r.contains(pos)){
        return true;
    }

    return false;
}
void MediaChooserButton::buttonPressed(){
    showPopup();
}

void MediaChooserButton::popupClosed(){
    Q_ASSERT(sender()==_popup);
    QLoggingCategory fcIo("ui.mediaChooser");
    qCDebug(fcIo) << "popupClose" ;
    setDown(false);
}

void MediaChooserButton::showPopup(){
    QLoggingCategory fcIo("ui.mediaChooser");
    qCDebug(fcIo) << "showPopup";

    if (_popup && !_popup->isVisible()){

        QPoint p2=mapToGlobal(QPoint(0,0));
        QSize menuSize = _popup->size();
        if (p2.y()-menuSize.height() > 0){
            _popup->move(QPoint(p2.x(),p2.y()-menuSize.height()));
        }

        _popup->popup(this,m_popupFilter);
        setDown(true);
    }
}

void MediaChooserButton::mediaSelected(const MediaChoosed &mediaChoosed){
    Q_ASSERT(sender()==_popup);


    emit mediaSelected2(mediaChoosed);
}

void MediaChooserButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && hitEraseButton(e->pos())) {
        e->accept();
        _mediaScaled = QPixmap();
        update();
      //  mediaSelected(QUrl());
        return;
    }

    QAbstractButton::mousePressEvent(e);
}

void MediaChooserButton::setPopup( MediaChooserPopup* popup, QFlags<ImageType> filter){

    if (popup == _popup){
        return;
    }

    if (popup && !_popup) {
        connect(this, SIGNAL(pressed()), this, SLOT(buttonPressed()), Qt::UniqueConnection);
    }


    if (_popup){
        disconnect(this, 0, _popup, 0);
    }

    _popup = popup;
    m_popupFilter = filter;

    update();
    updateGeometry();
}

void  MediaChooserButton::setMediaSize(const QSize& mediaSize){
    this->_mediaSize = mediaSize;
    setMedia(_media);
}

void  MediaChooserButton::setMedia(const QPixmap& media){
    this->_media = media;
    this->_mediaScaled=media.isNull()?QPixmap():media.scaled(_mediaSize, Qt::KeepAspectRatio,Qt::SmoothTransformation);
}

MediaChoosed::MediaChoosed(const QUrl& url) : _url(url){

}

MediaChoosed::MediaChoosed(const QString& localFilePath) : _localFilePath(localFilePath){
}

MediaChoosed::MediaChoosed(const ScraperResource &scraperResource): _scraperResource(scraperResource){
}

QUrl MediaChoosed::url()  const{
    return _url;
}

QString MediaChoosed::localFilePath() const{
    return _localFilePath;
}

ScraperResource MediaChoosed::scraperResource() const{
    return _scraperResource;
}

bool MediaChoosed::isMediaUrl() const{
    return !_url.isEmpty();
}

bool MediaChoosed::isMediaLocalFilePath() const{
    return !_localFilePath.isEmpty();
}

bool MediaChoosed::isMediaScraper() const{
    return !_scraperResource.isNull();
}

