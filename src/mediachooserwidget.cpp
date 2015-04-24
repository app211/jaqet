#include "mediachooserwidget.h"
#include "ui_mediachooserwidget.h"

#include <QGraphicsScene>
#include <QCloseEvent>
#include "mygraphicsobject.h"

#include <QDebug>

MediaChooserWidget::MediaChooserWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaChooserWidget)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    ui->setupUi(this);
}

MediaChooserWidget::~MediaChooserWidget()
{
    delete ui;
}

void MediaChooserWidget::setScene(QGraphicsScene* scene){
    ui->graphicsView->setScene(scene);
}


void MediaChooserWidget::closeEvent(QCloseEvent *event)
{
    emit WidgetClosed();
    event->accept();
}

void MediaChooserWidget::popup(Scraper::ImageType filter){

    int x=0;
    foreach (QGraphicsItem *item, ui->graphicsView->scene()->items(Qt::AscendingOrder))
    {
        if (item->type()==MyGraphicsObject::Type){
            MyGraphicsObject* m = static_cast<MyGraphicsObject*>(item);
            if (m->group()){
                if (m->imageType()==filter){
                    m->group()->setVisible(true);
                    m->group()->setX(x);
                    x+=210;
                } else {
                    m->group()->setVisible(false);
                }
            }
        }
    }
    show();
}
