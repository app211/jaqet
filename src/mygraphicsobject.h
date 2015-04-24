#ifndef MYGRAPHICSOBJECT_H
#define MYGRAPHICSOBJECT_H

#include "scrapers/scraper.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsItemGroup>

class MyGraphicsObject : public QGraphicsObject {

    QGraphicsPixmapItem* const m_p;
    QGraphicsItemGroup * m_group;
    const Scraper::ImageType m_type;

public :
    MyGraphicsObject(QGraphicsPixmapItem* p,  const Scraper::ImageType type) : QGraphicsObject(p), m_p(p), m_group(nullptr), m_type(type){

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

    Scraper::ImageType imageType(){
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
#endif // MYGRAPHICSOBJECT_H
