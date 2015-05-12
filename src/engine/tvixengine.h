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
    void proceed();
    QSize getBackdropSize() const;
    QSize getPosterSize() const;
    QSize getBannerSize() const;

    void preview(const QModelIndex & index);
    void preview(const CurrentItemData& data);

    void init(const QString& path);
protected:
    virtual TypeItem getTypeItem(const QFileInfo &f) const;

private slots:
    void previewOk(QPixmap result);


private:
    QGraphicsScene result;
    TemplateYadis b;

};

#endif // TVIXENGINE_H
