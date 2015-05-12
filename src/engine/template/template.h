#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QVariant>
#include "scrapers/scraper.h"
#include "scanner/mediainfo.h"

class Template : public QObject
{
    Q_OBJECT


public:
    explicit Template(QObject *parent = 0);

    void create(const CurrentItemData &data);
    virtual void proceed()=0;
    virtual QSize getBackdropSize() const=0;
    virtual QSize getPosterSize() const=0;
    virtual QSize getBannerSize() const=0;



protected:


   virtual void internalCreate(const CurrentItemData& data)=0;

};


#endif // TEMPLATE_H
