#include "engine.h"

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

Engine::TypeItem Engine::getTypeItem( const QModelIndex & index) const{
}

QFileInfo Engine::fileInfo(const QModelIndex & index) const {

    if (index.isValid()){
        return QFileInfo(currentDir,data(index,Qt::DisplayRole).toString());
    }

    return QFileInfo();
}

void Engine::populate(){
    currentDir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    currentDir.setNameFilters(getVisibleFileExtensions());
    QStringList l = currentDir.entryList();

    QDir up=currentDir;
    if (up.cdUp()){
        l.insert(0,"..");
    }

    this->setStringList(l);
}

Qt::ItemFlags Engine::flags ( const QModelIndex & index ) const{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
}

