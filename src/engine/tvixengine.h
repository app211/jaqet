#ifndef TVIXENGINE_H
#define TVIXENGINE_H

#include "fileengine.h"
#include "template/templateyadis.h"

class TVIXEngine : public FileEngine
{
    Q_OBJECT

    QStringList visibleFileExtensions;
public:
    TVIXEngine(QObject *parent = 0, const QString& path="");
    virtual QStringList getVisibleFileExtensions() const;
    void proceed(const CurrentItemData& data);
    QSize getBackdropSize() const;
    QSize getPosterSize() const;
    QSize getBannerSize() const;

    QGraphicsScene& preview(const QModelIndex & index) override;
    QGraphicsScene& preview(const CurrentItemData& data) override;

    QGraphicsScene& poster(const QModelIndex & index) override ;
    QGraphicsScene& poster(const CurrentItemData& data) override;

    void init(const QString& path);
protected:
    virtual TypeItem getTypeItem(const QFileInfo &f) const;

private:
    QGraphicsScene m_previewScene;
    QGraphicsScene m_posterScene;

    TemplateYadis b;

};

#endif // TVIXENGINE_H
