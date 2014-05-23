#include "myqstringlistmodel.h"

void MyQStringListModel::cdUp(){
    if (currentDir.cdUp()){
        populate();
    }
}

void MyQStringListModel::cd(const QString& path){
    if (QFileInfo(currentDir,path).isDir()){
        currentDir.cd(path);
        populate();
    }
}

MyQStringListModel::TypeItem MyQStringListModel::getTypeItem( const QModelIndex & index) const{
    if (index.isValid()){
        QFileInfo f(currentDir,data(index,Qt::DisplayRole).toString());
        if (f.isDir()){
            return TypeItem::DIR;
        } else if (f.isFile()){
            return TypeItem::PROCEEDABLE;
        }
    }

    return TypeItem::UNKWON;
}


bool MyQStringListModel::isDir( const QModelIndex & index) const {

    if (index.isValid()){
        return QFileInfo(currentDir,data(index,Qt::DisplayRole).toString()).isDir();
    }

    return false;
}

QFileInfo MyQStringListModel::fileInfo(const QModelIndex & index) const {

    if (index.isValid()){
        return QFileInfo(currentDir,data(index,Qt::DisplayRole).toString());
    }

    return QFileInfo();
}

void MyQStringListModel::populate(){
    currentDir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    QStringList l = currentDir.entryList();

    QDir up=currentDir;
    if (up.cdUp()){
        l.insert(0,"..");
    }

    this->setStringList(l);
}

Qt::ItemFlags MyQStringListModel::flags ( const QModelIndex & index ) const{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
}

