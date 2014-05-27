#ifndef TVIXENGINE_H
#define TVIXENGINE_H

#include "engine.h"

class TVIXEngine : public Engine
{
    QStringList visibleFileExtensions;
public:
    TVIXEngine(QObject *parent = 0, const QString& path="");
    virtual TypeItem getTypeItem( const QModelIndex & index) const;
    virtual QStringList getVisibleFileExtensions() const;
};

#endif // TVIXENGINE_H
