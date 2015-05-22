#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <QGraphicsScene>
#include <QIcon>
#include <QFileInfo>

#include "template/template.h"
#include "scrapers/scraper.h"


class Engine : public QAbstractListModel {
    Q_OBJECT

public :

    enum class TypeItem {
        PROCEEDABLE, PROCEEDED, UNKWON, DIR
    };

    explicit Engine(QObject *parent = 0, const QString& path="");

    virtual bool canGiveFileInfo(){return false;}
    int rowCount(const QModelIndex &parent = QModelIndex()) const=0;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const=0;
    Qt::ItemFlags flags(const QModelIndex &index) const=0;

    virtual TypeItem getTypeItem(const QModelIndex & index) const=0;
    virtual QStringList getVisibleFileExtensions() const = 0;
    virtual QFileInfo getFileInfo(const QModelIndex &){return QFileInfo();}
    virtual void internalDoubleClicked ( const QModelIndex & index )=0;

    virtual QGraphicsScene& preview(const CurrentItemData& data)=0;

    virtual void proceed(const CurrentItemData& data)=0;

    virtual QSize getBackdropSize() const=0;
    virtual QSize getPosterSize() const =0;


protected:
    QIcon bingo;

    virtual void populate()=0;
    virtual TypeItem getTypeItem(const QFileInfo &f) const=0;

private :


private slots:
     void doubleClicked ( const QModelIndex & index );

signals :
    void previewOK(QGraphicsScene*);

};

#endif // MYQSTRINGLISTMODEL_H
