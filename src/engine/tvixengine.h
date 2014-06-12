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
    virtual QGraphicsScene* preview(const QFileInfo &f);
    void create(const QMap<Template::Properties, QVariant> &newproperties);
    void proceed();
    QSize getBackdropSize() const;
    QSize getPosterSize() const;
    void preview(const QMap<Template::Properties, QVariant> &newproperties);

protected:
    virtual TypeItem getTypeItem(const QFileInfo &f) const;

private slots:
    void previewOk(QPixmap result);

signals:
    void previewOK(QGraphicsScene*);

private:
    QGraphicsScene result;
    TemplateYadis b;

};

#endif // TVIXENGINE_H
