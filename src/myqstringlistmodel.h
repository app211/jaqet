#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QStringListModel>
#include <QDir>


class MyQStringListModel : public QStringListModel {
public :

    QDir currentDir;

    explicit MyQStringListModel(QObject *parent = 0, const QString& path=""):
        QStringListModel(parent){
        cd(path);
    }

    void cdUp();
    void cd(const QString& path);
    bool isDir( const QModelIndex & index) const;
    QFileInfo fileInfo(const QModelIndex & index)const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;

private :
    void populate();

};

#endif // MYQSTRINGLISTMODEL_H
