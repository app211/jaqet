#include "engine.h"
#include <QDebug>

 Engine::Engine(QObject *parent, const QString& path
                ):
    QAbstractListModel(parent){
    bingo.addFile(":/resources/images/bingo16x16.png",QSize(16,16));
    bingo.addFile(":/resources/images/bingo32x32.png",QSize(32,32));


#ifdef Q_OS_WIN32
    b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");
#else
    b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    // b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");

    // b.loadTemplate("/home/teddy/Developpement/Maxx Shiny/template.xml");
#endif

    connect(&b, SIGNAL(tivxOk(QPixmap )), this, SLOT(resultOk(QPixmap )));

}

void Engine::resultOk(QPixmap result){
    emit tivxOk(result);
}

void Engine::create(const QMap<Template::Properties, QVariant> &newproperties){
    b.create(newproperties);
}

void Engine::proceed(){
    b.proceed();
    populate();
}

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
