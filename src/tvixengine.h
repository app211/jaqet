#ifndef TVIXENGINE_H
#define TVIXENGINE_H

#include "engine.h"

class TVIXEngine : public Engine
{
    QStringList visibleFileExtensions;
public:
    TVIXEngine(QObject *parent = 0, const QString& path="");
     virtual QStringList getVisibleFileExtensions() const;
    virtual QGraphicsScene* preview(const QFileInfo &f);
protected:
    virtual TypeItem getTypeItem(const QFileInfo &f) const;

private:
     QGraphicsScene result;
};

#endif // TVIXENGINE_H
