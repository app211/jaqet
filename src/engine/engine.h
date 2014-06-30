#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <QGraphicsScene>
#include <QIcon>
#include <QFileInfo>

#include "template/template.h"


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
    virtual QFileInfo getFileInfo(const QModelIndex &index){return QFileInfo();}
    virtual void internalDoubleClicked ( const QModelIndex & index )=0;

    virtual void preview(const QMap<Template::Properties, QVariant> &newproperties, bool reset=false)=0;
    virtual void preview(const QModelIndex & index)=0;

    virtual void proceed()=0;

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
