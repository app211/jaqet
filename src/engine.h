#ifndef MYQSTRINGLISTMODEL_H
#define MYQSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QFileIconProvider>
#include <QGraphicsScene>

class Engine : public QAbstractListModel {
public :


    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const ;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    enum class TypeItem {
        PROCEEDABLE, PROCEEDED, UNKWON, DIR
    };

    QDir currentDir;

    explicit Engine(QObject *parent = 0, const QString& path=""):
        QAbstractListModel(parent){
        bingo.addFile(":/resources/images/bingo16x16.png",QSize(16,16));
        bingo.addFile(":/resources/images/bingo32x32.png",QSize(32,32));
    }

    void cdUp();
    void cd(const QString& path);
    QFileInfo fileInfo(const QModelIndex & index)const;

    TypeItem getTypeItem(const QModelIndex & index) const;
    virtual QStringList getVisibleFileExtensions() const = 0;
    virtual QGraphicsScene* preview(const QFileInfo &f)=0;

protected:
    QIcon bingo;
    virtual TypeItem getTypeItem(const QFileInfo &f) const=0;

private :
    void populate();
    QFileInfoList entryInfoList;
    bool allowUp=false;
    QFileIconProvider iconProvider;
    static bool lessThan(const QFileInfo &s1, const QFileInfo &s2);


};

#endif // MYQSTRINGLISTMODEL_H
