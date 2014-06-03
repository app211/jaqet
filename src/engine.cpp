#include "engine.h"
#include <QDebug>

int Engine::rowCount(const QModelIndex &parent) const {
    return entryInfoList.size()+(allowUp?1:0);
}

Qt::ItemFlags Engine::flags(const QModelIndex &index) const{
    if(!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsSelectable;
}

QVariant Engine::data(const QModelIndex &index, int role) const {
    if(!index.isValid()){
        return QVariant();
    }
    if(role == Qt::DisplayRole)
    {
        if (allowUp){
            return ((index.row()==0)?"..":entryInfoList.at(index.row()-1).fileName());
        } else {
            return entryInfoList.at(index.row()).fileName();
        }
    }

    if(role == Qt::DecorationRole)
    {
        if (allowUp && index.row()==0){
            return iconProvider.icon(QFileIconProvider::Folder);
        }

        QFileInfo info=(entryInfoList.at(index.row()-(allowUp?1:0)));

        if (this->getTypeItem(info)==TypeItem::PROCEEDED){
            return bingo;
        } else {

            if(info.exists())
                return iconProvider.icon(info);
            else
                return iconProvider.icon(QFileIconProvider::File);
        }
    }

    return QVariant();
}

void Engine::cdUp(){
    if (currentDir.cdUp()){
        populate();
    }
}

void Engine::cd(const QString& path){
    if (QFileInfo(currentDir,path).isDir()){
        currentDir.cd(path);
        populate();
    }
}

QFileInfo Engine::fileInfo(const QModelIndex & index) const {

    if (index.isValid() && !(allowUp && index.row()==0)){
        return entryInfoList.at(index.row()-(allowUp?1:0));
    }

    return QFileInfo();
}

Engine::TypeItem  Engine::getTypeItem(const QModelIndex & index) const {
    if (index.isValid()){
        if ((allowUp && index.row()==0)){
            return TypeItem::DIR;
        } else {
            return getTypeItem(entryInfoList.at(index.row()-(allowUp?1:0)));
        }

    }
    return TypeItem::UNKWON;
}

void Engine::populate(){
    emit beginResetModel();

    currentDir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    currentDir.setNameFilters(getVisibleFileExtensions());

    entryInfoList =currentDir.entryInfoList();

    std::sort(entryInfoList.begin(), entryInfoList.end(), lessThan);

    allowUp= (QDir(currentDir).cdUp());

    emit endResetModel();
}

bool Engine::lessThan(const QFileInfo &s1, const QFileInfo &s2)
{
    return (s1.isDir() && !s2.isDir()) || (s1.isDir()==s2.isDir() && s1.fileName().compare(s2.fileName(),Qt::CaseInsensitive) < 0);
}
