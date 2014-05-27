#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QStringListModel>
#include <QDir>


class Engine : public QStringListModel {
public :

    enum class TypeItem {
        PROCEEDABLE, PROCEEDED, UNKWON, DIR
    };

    QDir currentDir;

    explicit Engine(QObject *parent = 0, const QString& path=""):
        QStringListModel(parent){
    }

    void cdUp();
    void cd(const QString& path);

    virtual TypeItem getTypeItem( const QModelIndex & index) const=0;
    virtual QStringList getVisibleFileExtensions() const = 0;

    QFileInfo fileInfo(const QModelIndex & index)const;

private :
    void populate();
    Qt::ItemFlags flags ( const QModelIndex & index ) const;

};

#endif // MYQSTRINGLISTMODEL_H
