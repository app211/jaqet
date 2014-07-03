#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QVariant>

class Template : public QObject
{
    Q_OBJECT

public:
    explicit Template(QObject *parent = 0);

    enum class Properties {
        tv,actors,banner,director,synopsis,year,runtime,title,poster,backdrop,mediainfo,fileinfo,originaltitle,season,episode,episodetitle
    };


    void create(const QMap<Properties, QVariant> &newproperties, bool reset);
    virtual void proceed()=0;
    virtual QSize getBackdropSize() const=0;
    virtual QSize getPosterSize() const=0;
    virtual QSize getBannerSize() const=0;

protected:

    QMap<Properties, QVariant> properties;

    bool updateProperties(const QMap<Properties, QVariant> &newProperties);

    virtual void internalCreate()=0;

};


#endif // TEMPLATE_H
