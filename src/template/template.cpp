#include "template.h"

Template::Template(QObject *parent) :
    QObject(parent)
{
}

void Template::create(const QMap<Properties, QVariant> &newproperties){
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
        this->properties[i.key()]=i.value();
        somethingChanged=true;
     }

    return somethingChanged;
}
