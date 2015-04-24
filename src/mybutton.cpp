#include "mybutton.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QDebug>
#include "mediachooserwidget.h"


MyButton::MyButton(QWidget *parent) :
    QAbstractButton(parent),
    _popup(nullptr)
{
}

void MyButton::initStyleOption(QStyleOptionButton *option) const
{
    if (!option)
        return;

    //  Q_D(const QPushButton);
    option->initFrom(this);
    option->features = QStyleOptionButton::None;
    option->features |= QStyleOptionButton::Flat;

    if (_popup) {
        option->features |= QStyleOptionButton::HasMenu;
    }

    if (isDown() /*|| d->menuOpen*/)
        option->state |= QStyle::State_Sunken;
    if ( !isDown())
        option->state |= QStyle::State_Raised;


     option->text = QString::null;

     option->icon = this->icon();
     option->iconSize = QSize(50,50);
}

void MyButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);
}

void MyButton::_q_popupPressed(){
    showPopup();
}

void MyButton::popupClose(){
    qDebug() << "popupClose";
    setDown(false);
}

void MyButton::showPopup(){
    qDebug() << "showPopup";

    if (_popup){
        QPoint p2=mapToGlobal(QPoint(0,0));
        QSize menuSize = _popup->size();
        if (p2.y()-menuSize.height() > 0){
            _popup->move(QPoint(p2.x(),p2.y()-menuSize.height()));
        }

        _popup->popup(_filter);
        setDown(true);
    }
}


void MyButton::setPopup( MediaChooserWidget* popup, Scraper::ImageType filter){

    if (popup == _popup){
        return;
    }

    if (popup && !_popup) {
        connect(this, SIGNAL(pressed()), this, SLOT(_q_popupPressed()), Qt::UniqueConnection);
    }


    if (_popup){
        disconnect(this, 0, _popup, 0);
    }

    _popup = popup;
    _filter = filter;

    if (_popup){
        connect(_popup, SIGNAL(WidgetClosed (  )), this, SLOT(popupClose( )), Qt::UniqueConnection);
     }

    update();
    updateGeometry();
}

void MyButton::myfocusOutEvent ( QFocusEvent * event ) {
   qDebug() << event;
}
