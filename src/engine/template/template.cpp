#include "template.h"
#include <QDebug>

Template::Template(QObject *parent) :
    QObject(parent)
{
}

QPixmap Template::create(const CurrentItemData& data){
     return internalCreate(data);
}
