#include "fileengine.h"

#include <QDebug>

bool lessThan(const QFileInfo &s1, const QFileInfo &s2)
{
    return (s1.isDir() && !s2.isDir()) || (s1.isDir()==s2.isDir() && s1.fileName().compare(s2.fileName(),Qt::CaseInsensitive) < 0);
}

void FileEngine::internalDoubleClicked ( const QModelIndex & index ){
    if (this->getTypeItem(index)==TypeItem::DIR){
        if ((allowUp && index.row()==0)){
            cdUp();
        } else {
            cd(fileInfo(index).absoluteFilePath());
        }
    }
}

FileEngine::FileEngine(QObject *parent, const QString& path) :
    Engine(parent)
{
}

void FileEngine::init(const QString& path)
{
    connect(&m, SIGNAL(directoryChanged(const QString & )), this,
            SLOT(directoryChanged(const QString &)));

    connect(&m, SIGNAL(fileChanged(const QString &)), this,
            SLOT(fileChanged(const QString &)));

    cd(path);
}


int FileEngine::rowCount(const QModelIndex &parent) const {
    return entryInfoList.size()+(allowUp?1:0);
}

Qt::ItemFlags FileEngine::flags(const QModelIndex &index) const{
    if(!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsSelectable;
}

QVariant FileEngine::data(const QModelIndex &index, int role) const {
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

void FileEngine::cdUp(){
    m.disabled();
    if (currentDir.cdUp()){
        m.setPath(currentDir.absolutePath());
        populate();
    }

    m.enabled();
}

void FileEngine::cd(const QString& path){
    m.disabled();

    if (QFileInfo(currentDir,path).isDir()){
        currentDir.cd(path);
        m.setPath(currentDir.absolutePath());
        populate();
    }

    m.enabled();
}

QFileInfo FileEngine::fileInfo(const QModelIndex & index) const {

    if (index.isValid() && !(allowUp && index.row()==0)){
        return entryInfoList.at(index.row()-(allowUp?1:0));
    }

    return QFileInfo();
}

Engine::TypeItem  FileEngine::getTypeItem(const QModelIndex & index) const {
    if (index.isValid()){
        if ((allowUp && index.row()==0)){
            return TypeItem::DIR;
        } else {
            return getTypeItem(entryInfoList.at(index.row()-(allowUp?1:0)));
        }

    }
    return TypeItem::UNKWON;
}

void FileEngine::populate(){
    emit beginResetModel();

    currentDir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    currentDir.setNameFilters(getVisibleFileExtensions());

    entryInfoList =currentDir.entryInfoList();

    std::sort(entryInfoList.begin(), entryInfoList.end(), lessThan);

    allowUp= (QDir(currentDir).cdUp());

    emit endResetModel();
}

void FileEngine::directoryChanged(const QString & path){
    populate();
    qDebug() << path;
}

void FileEngine::fileChanged(const QString & path){
    qDebug() << path;
}
