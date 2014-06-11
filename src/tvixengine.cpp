#include "tvixengine.h"
#include <QGraphicsPixmapItem>

TVIXEngine::TVIXEngine(QObject *parent, const QString& path):
    Engine(parent,path){

    visibleFileExtensions = { "*.asf"
                              ,"*.avi"
                              ,"*.dat"
                              ,"*.divx"
                              ,"*.ifo"
                              ,"*.iso"
                              ,"*.m2v"
                              ,"*.mkv"
                              ,"*.mpeg"
                              ,"*.mpg"
                              ,"*.tp"
                              ,"*.trp"
                              ,"*.ts"
                              ,"*.vob"
                              ,"*.wmv"
                              ,"*.wvx"
                              ,"*.xvid"
                             /* ,"*.mp3"
                              ,"*.ogg"
                              ,"*.wav"*/
                              ,"*.wma"
                            };
    cd(path);


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

QGraphicsScene *TVIXEngine::preview(const QFileInfo &f) {
    result.clear();

    if (getTypeItem(f)!=TypeItem::PROCEEDED){

    } else {
        QFileInfo posterFileinfo(QDir(f.absoluteFilePath()),"folder.jpg");
        QPixmap poster;
        if (posterFileinfo.exists()){
            if (poster.load(posterFileinfo.absoluteFilePath())){
                result.addPixmap(poster);
            }
        }

        QFileInfo imageFileinfo(QDir(f.absoluteFilePath()),"tvix.jpg");
        QPixmap image;
        if (imageFileinfo.exists()){
            if (image.load(imageFileinfo.absoluteFilePath())){
                QGraphicsPixmapItem *p= result.addPixmap(image);
                if (!poster.isNull()){
                    p->setPos(0,poster.height());
                }
            }
        }
    }

    return &result;
}
