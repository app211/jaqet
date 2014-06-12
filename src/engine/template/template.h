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
        actors,director,synopsis,year,runtime,title,poster,backdrop,mediainfo,fileinfo,originaltitle
    };

    QSize getBackdropSize() const{
        return backdropSize;
    }

    QSize getPosterSize() const{
        return posterSize;
    }

    void create(const QMap<Properties, QVariant> &newproperties);
    virtual void proceed()=0;

protected:
    QSize backdropSize;
    QSize posterSize;

    QMap<Properties, QVariant> properties;

    bool updateProperties(const QMap<Properties, QVariant> &newProperties);

    virtual void internalCreate()=0;

};


#endif // TEMPLATE_H
