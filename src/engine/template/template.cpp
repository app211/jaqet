#include "template.h"
#include <QDebug>

Template::Template(QObject *parent) :
    QObject(parent)
{
}

void Template::create(const CurrentItemData& data){
     internalCreate(data);
}
