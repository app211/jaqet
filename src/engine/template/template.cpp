#include "template.h"
#include <QDebug>

Template::Template(QObject *parent) :
    QObject(parent)
{
}

void Template::create(const QMap<Properties, QVariant> &newproperties, bool reset){
    if (reset){
        properties.clear();
    }

    if (!updateProperties(newproperties)){
        return;
    }

    internalCreate();
}

bool Template::updateProperties(const QMap<Template::Properties, QVariant>& newProperties){
    bool somethingChanged=false;
    QMapIterator<Template::Properties, QVariant> i(newProperties);
    while (i.hasNext()) {
        i.next();
        properties[i.key()]=i.value();
        qDebug() << (int)i.key() << i.value().toString();
        somethingChanged=true;

    }

    return somethingChanged;
}
