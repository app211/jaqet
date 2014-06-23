#include "tvixengine.h"
#include <QGraphicsPixmapItem>

TVIXEngine::TVIXEngine(QObject *parent, const QString& path):
    FileEngine(parent, path){


    init(path);

}

void TVIXEngine::init(const QString& path){
#ifdef Q_OS_WIN32
    b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");
#else
    b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    // b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");

    // b.loadTemplate("/home/teddy/Developpement/Maxx Shiny/template.xml");
#endif

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

void TVIXEngine::preview(const QMap<Template::Properties, QVariant> &newproperties){
    connect(&b, SIGNAL(tivxOk(QPixmap )), this, SLOT(previewOk(QPixmap )));
    create(newproperties);
}


void TVIXEngine::previewOk(QPixmap pimap){

    result.clear();
    result.addPixmap(pimap);
    emit previewOK(&result);
}

void TVIXEngine::preview(const QModelIndex & index){
    QFileInfo f=fileInfo(index);

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

   emit previewOK(&result);
}

void TVIXEngine::create(const QMap<Template::Properties, QVariant> &newproperties){
     b.create(newproperties);
}

void TVIXEngine::proceed(){
    b.proceed();
    populate();
}

QSize TVIXEngine::getBackdropSize() const{
    return b.getBackdropSize();
}

QSize TVIXEngine::getPosterSize() const {
    return b.getPosterSize();
}