#include "tvixengine.h"

TVIXEngine::TVIXEngine(QObject *parent, const QString& path):
    Engine(parent,path){

    visibleFileExtensions  << "*.asf" << "*.avi" << "*.dat"   << "*.divx"  << "*.ifo"
                           << "*.iso"
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
                 <<"*.xvid"
                <<"*.mp3"
               <<"*.ogg"
              <<"*.wav"
             <<"*.wma";
    cd(path);

}

Engine::TypeItem TVIXEngine::getTypeItem( const QModelIndex & index) const {
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

QStringList TVIXEngine::getVisibleFileExtensions() const {
    return visibleFileExtensions;
}

