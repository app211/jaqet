#include "tvixengine.h"
#include <QGraphicsPixmapItem>

TVIXEngine::TVIXEngine(QObject *parent, const QString& path):
    FileEngine(parent, path){
    init(path);
}

void TVIXEngine::init(const QString& path){
    b.loadTemplate(":/resources/templates/CryFibril/template.xml");

    // b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");

  //  b.loadTemplate("/home/teddy/developpement/Filmoscope/template.xml");

    visibleFileExtensions << "*.asf"
                          <<"*.avi"
                         <<"*.dat"
                        <<"*.divx"
                       <<"*.ifo"
                      <<"*.iso"
                     <<"*.m2v"
                    <<"*.mkv"
                   <<"*.mpeg"
                  <<"*.mpg"
                 <<"*.tp"
                <<"*.trp"
               <<"*.ts"
              <<"*.vob"
             <<"*.wmv"
            <<"*.wvx"
           <<"*.xvid"<<
             "*.mp4"
          <<"*.wma";

    FileEngine::init(path);

}

Engine::TypeItem TVIXEngine::getTypeItem( const QFileInfo & f) const {
    if (f.exists())
    {
        if (f.isDir()){
            bool b;
            b=QFileInfo(QDir(f.absoluteFilePath()),"tvix.jpg").exists()
                    && QFileInfo(QDir(f.absoluteFilePath()),"folder.jpg").exists();
            if (b){
                return TypeItem::PROCEEDED;
            } else {
                return TypeItem::DIR;
            }
        } else if (f.isFile()){
            return TypeItem::PROCEEDABLE;
        }
    }

    return TypeItem::UNKWON;

}

QStringList TVIXEngine::getVisibleFileExtensions() const {
    return visibleFileExtensions;
}

QGraphicsScene &TVIXEngine::preview(const CurrentItemData &data){
    m_previewScene.clear();
    m_previewScene.addPixmap(b.create(data));
    return m_previewScene;
}

QGraphicsScene &TVIXEngine::preview(const QModelIndex & index){
    QFileInfo f=fileInfo(index);

    m_previewScene.clear();

    if (getTypeItem(f)==TypeItem::PROCEEDED){
        QGraphicsPixmapItem *preview=nullptr;

        QFileInfo imageFileinfo(QDir(f.absoluteFilePath()),"tvix.jpg");
        if (imageFileinfo.exists()){
            QPixmap image;
            if (image.load(imageFileinfo.absoluteFilePath())){
                preview= m_previewScene.addPixmap(image);
            }
        }

        QFileInfo posterFileinfo(QDir(f.absoluteFilePath()),"folder.jpg");
        if (posterFileinfo.exists()){
            QPixmap poster;
            if (poster.load(posterFileinfo.absoluteFilePath())){
                 QGraphicsPixmapItem *poster2=m_previewScene.addPixmap(poster);
                 if (preview != nullptr){
                     poster2->setPos(preview->pos()+QPoint(preview->boundingRect().width(),0));
                 }
            }
        }
    }

    return m_previewScene;
}
/*
QGraphicsScene& TVIXEngine::poster(const QModelIndex & index)  {
    QFileInfo f=fileInfo(index);

    m_posterScene.clear();

    if (getTypeItem(f)==TypeItem::PROCEEDED){

        QFileInfo posterFileinfo(QDir(f.absoluteFilePath()),"folder.jpg");
        QPixmap poster;
        if (posterFileinfo.exists()){
            if (poster.load(posterFileinfo.absoluteFilePath())){
                m_posterScene.addPixmap(poster);
            }
        }
    }

    return m_posterScene;
}

QGraphicsScene& TVIXEngine::poster(const CurrentItemData& data) {
    m_posterScene.clear();
    QPixmap poster=data.getPoster();
    if (!poster.isNull()){
        m_posterScene.addPixmap(poster);
    }
    return m_posterScene;
}
*/
void TVIXEngine::proceed(const CurrentItemData& data){
    b.proceed(data);
    populate();
}

QSize TVIXEngine::getBackdropSize() const{
    return b.getBackdropSize();
}

QSize TVIXEngine::getPosterSize() const {
    return b.getPosterSize();
}

QSize TVIXEngine::getBannerSize() const {
    return b.getBannerSize();
}
