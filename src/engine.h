#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QFileIconProvider>
#include <QGraphicsScene>
#include "template/templateyadis.h"

class Engine : public QAbstractListModel {
    Q_OBJECT

public :


    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const ;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    enum class TypeItem {
        PROCEEDABLE, PROCEEDED, UNKWON, DIR
    };

    QDir currentDir;

    explicit Engine(QObject *parent = 0, const QString& path="");

    void cdUp();
    void cd(const QString& path);
    QFileInfo fileInfo(const QModelIndex & index)const;

    TypeItem getTypeItem(const QModelIndex & index) const;
    virtual QStringList getVisibleFileExtensions() const = 0;
    virtual QGraphicsScene* preview(const QFileInfo &f)=0;

    void create(const QMap<Template::Properties, QVariant> &newproperties);
    void proceed();

    QSize getBackdropSize() const{
        return b.getBackdropSize();
    }

    QSize getPosterSize() const {
        return b.getPosterSize();
    }
protected:
    QIcon bingo;
    virtual TypeItem getTypeItem(const QFileInfo &f) const=0;

private :
    void populate();
    QFileInfoList entryInfoList;
    bool allowUp=false;
    QFileIconProvider iconProvider;
    static bool lessThan(const QFileInfo &s1, const QFileInfo &s2);
    TemplateYadis b;

private slots:
    void resultOk(QPixmap result);

signals :
    void tivxOk(QPixmap result);
};

#endif // MYQSTRINGLISTMODEL_H
