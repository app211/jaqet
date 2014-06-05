#include "templateyadis.h"

#include <QXmlStreamReader>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QFont>
#include <QSet>
#include <QFontDatabase>
#include <QBitmap>
#include <QPixmap>
#include "../scrapers/scraper.h"
#include <QNetworkReply>



TemplateYadis::TemplateYadis()
{
}

void TemplateYadis::parseMoviePoster(const QDomElement& e){
    QDomElement standard=e.firstChildElement("standard");
    if (!standard.isNull()){
        QDomElement size=standard.firstChildElement("size");
        if (!size.isNull()){
            poster_standard_width=size.attribute("width");
            poster_standard_height=size.attribute("height");
            poster_standard_border=size.attribute("border");
        }
        
        QDomElement mask=standard.firstChildElement("mask");
        if (!mask.isNull()){
            poster_standard_mask=mask.text();
        }
        
        QDomElement frame=standard.firstChildElement("frame");
        if (!frame.isNull()){
            poster_standard_frame=frame.text();
        }
        
    }
    
}


void TemplateYadis::parseMovie(const QDomElement& e){
    
    if (e.hasAttribute("background")){
        movieBackground = e.attribute("background");
    }
    
    QDomNode n = e.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="poster"){
                parseMoviePoster(e);
            }else if (e.tagName()=="synopsis"){
                parseSynopsis(e);
            }
        }
        
        
        n = n.nextSibling();
    }
}

QString TemplateYadis::getAbsoluteFilePath(const QString& fileName){
#ifndef Q_OS_WIN
    static bool isCaseSensitive=true;
#else
    static bool isCaseSensitive=false;
#endif
    QString absoluteFilename= QFileInfo(QDir(absoluteTemplateFilePath),fileName).absoluteFilePath();
    if (isCaseSensitive){
        // Template come from an unsensitive file system... On sensitive system, we have sometimes to correct the filename
        QFileInfo fileInfo(absoluteFilename);
        if (!fileInfo.exists()){
            QDir parent_dir = fileInfo.absoluteDir();
            QStringList filters;
            filters << fileInfo.fileName();
            QStringList list=parent_dir.entryList(filters,QDir::Files | QDir::Hidden | QDir::NoSymLinks);
            if (list.size()==1){
                return QFileInfo(parent_dir,list.at(0)).absoluteFilePath();
            }
        }
    }
    return absoluteFilename;
}

bool TemplateYadis::parseImage(const QDomElement& imageElement, templateYadis_image& image){
    if (!imageElement.hasAttribute("type")){
        return false;
    }
    
    image.type = imageElement.attribute("type");
    
    if (!imageElement.hasAttribute("x")){
        return false;
    }
    
    bool bOk;
    image.x=imageElement.attribute("x").toInt(&bOk);
    if (!bOk){
        return false;
    }
    image.y=imageElement.attribute("y").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    image.w=imageElement.attribute("width").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    image.h=imageElement.attribute("height").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    image.value=imageElement.text();
    
    return true;
}

bool TemplateYadis::parseText(const QDomElement& textElement, templateYadis_text& text){
    if (!textElement.hasAttribute("type")){
        return false;
    }
    
    text.type = textElement.attribute("type");
    text.color = textElement.attribute("color");
    text.language = textElement.attribute("language");
    text.font = textElement.attribute("font");
    
    if (!textElement.hasAttribute("x")){
        return false;
    }
    
    bool bOk;
    if (textElement.hasAttribute("size")){
        
        text.size=textElement.attribute("size").toInt(&bOk);
        if (!bOk){
            return false;
        }
        
    }
    
    text.x=textElement.attribute("x").toInt(&bOk);
    if (!bOk){
        return false;
    }
    text.y=textElement.attribute("y").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    text.w=textElement.attribute("width").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    text.h=textElement.attribute("height").toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    text.value=textElement.text();
    
    return true;
}

void TemplateYadis::parseSynopsis(const QDomElement& synopsisNode){
    QDomNode n = synopsisNode.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="image"){
                templateYadis_image image;
                parseImage(e,image);
                movie_synopsis_images.append(image);
            }else if (e.tagName()=="text"){
                templateYadis_text text;
                parseText(e,text);
                movie_synopsis_texts.append(text);
            }
        }
        
        
        n = n.nextSibling();
    }
}

QSize TemplateYadis::getSize(){
    int w=0;
    int h=0;

    foreach (const templateYadis_image& image, movie_synopsis_images){
        w=qMax<int>(w,image.w);
        h=qMax<int>(h,image.h);

    }

    return QSize(w,h);
}

bool TemplateYadis::buildPoster(const ScraperResource& poster, QPixmap& pixmap){
    
    bool bOk;
    int w=poster_standard_width.toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    int h=poster_standard_height.toInt(&bOk);
    if (!bOk){
        return false;
    }
    
    QPixmap result(w,h);
    result.fill(Qt::transparent);
    QPainter pixPaint(&result);
    
    
    /*
    QPixmap pposter;
    loadPixmap(poster.scraper->getBestImageUrl(poster.resources,QSize(w,h)),pposter);
    pposter=pposter.scaled(QSize(w,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    
    if (!poster_standard_mask.isEmpty()){
        QPixmap mask;
        if (!mask.load(getAbsoluteFilePath(poster_standard_mask))){
            return false;
        }
        
        pposter.setMask(mask.scaled(QSize(w,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).mask());
    }
    
    pixPaint.drawPixmap(0,0,w,h,pposter);
    
    if (!poster_standard_frame.isEmpty()){
        QPixmap frame;
        if (!frame.load(getAbsoluteFilePath(poster_standard_frame))){
            return false;
        }
        
        pixPaint.drawPixmap(0,0,w,h,frame);
    }
    */
    pixmap=result;
    
    return true;
}

void TemplateYadis::create(const QPixmap &poster, const QPixmap &backdrop, QMap<QString, QString> texts, const MediaInfo& mediaInfo){

        int w=0;
        int h=0;

        foreach (const templateYadis_image& image, movie_synopsis_images){
            w=qMax<int>(w,image.w);
            h=qMax<int>(h,image.h);

        }

        QPixmap result(w,h);
        result.fill(Qt::transparent);
        QPainter pixPaint(&result);

        //pixPaint.setBackgroundMode(Qt::OpaqueMode);
        foreach (const templateYadis_image& image, movie_synopsis_images){
            qDebug() << image.type << image.value;
            if (image.type=="fanart" && !backdrop.isNull()){
                QPixmap scaled=backdrop.scaled(QSize(image.w,image.h),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                int x= image.x+ (image.w-scaled.width())/2;
                int y= image.y+ (image.h-scaled.height())/2;
                int w= scaled.width();
                int h=scaled.height();
                pixPaint.drawPixmap(x,y,w,h,scaled);

            }else if (image.type=="poster" && !poster.isNull()){
                QPixmap scaled=poster.scaled(QSize(image.w,image.h),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                int x= image.x+ (image.w-scaled.width())/2;
                int y= image.y+ (image.h-scaled.height())/2;
                int w= scaled.width();
                int h=scaled.height();
                pixPaint.drawPixmap(x,y,w,h,scaled);

            } else if (image.type=="static" && !image.value.isEmpty()){
                QPixmap pstatic;

                if (pstatic.load(getAbsoluteFilePath(image.value)) ){
                    pixPaint.drawPixmap(image.x,image.y,image.w,image.h,pstatic);
                }
            }

        }

        foreach (const templateYadis_text& text, movie_synopsis_texts){
            if (text.type=="static" && !text.value.isEmpty() && (text.language.isEmpty() ||text.language=="fr")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,0,text.value);

            } else if (text.type=="plot" && texts.contains("synopsis")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["synopsis"]);

            } else if (text.type=="cast" && texts.contains("actors")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["actors"]);

            } else if (text.type=="director" && texts.contains("directors")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["directors"]);

            } else if (text.type=="year" && texts.contains("year")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["year"]);

            } else if (text.type=="runtime" && texts.contains("runtime")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["runtime"]);

            }else if (text.type=="title" && texts.contains("title")){
                QFont font(text.font);
                font.setPixelSize(text.size);
                pixPaint.setFont(font);
                pixPaint.setPen(QPen(QColor(text.color)));
                pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,texts["title"]);

            } else if (text.type=="resolution" && !mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).isNull()){
            QFont font(text.font);
            font.setPixelSize(text.size);
            pixPaint.setFont(font);
            pixPaint.setPen(QPen(QColor(text.color)));
            QSize v=mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).toSize();
            QString s=QString("%1 %2").arg(v.width()).arg(v.height());
            qDebug() << s;
            pixPaint.drawText(text.x,text.y,text.w,text.h,Qt::AlignLeft|Qt::TextWordWrap,s);

        }


        }

        emit tivxOk(result);

}

bool TemplateYadis::loadTemplate(const QString& fileName){
    
    
    QDomDocument doc("mydocument");
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)){
        return false;
    }
    
    absoluteTemplateFilePath=QFileInfo(file).absolutePath();
    
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    
    file.close();
    
    QDomElement docElem = doc.documentElement();
    
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (e.tagName()=="movie"){
                parseMovie(e);
            }
            
            qDebug() << e.tagName();
            if (e.tagName()=="title" && e.hasAttribute("text")){
                //    scrapFileFile.title = e.attribute("text");
            }
            else if (e.tagName()=="synopsis" && e.hasAttribute("text")){
                //    scrapFileFile.synopsis = e.attribute("text");
            }
            else if (e.tagName()=="release_date" && e.hasAttribute("text")){
                //     scrapFileFile.releaseDate = e.attribute("text");
            }
        }
        n = n.nextSibling();
    }
    
    QDir dir(absoluteTemplateFilePath);
    QStringList filters;
    filters << "*.ttf";
    
    foreach ( QString fontName, dir.entryList(filters, QDir::Files) ){
        QString fontPath =getAbsoluteFilePath(fontName);
        qDebug() << fontPath << QFontDatabase::addApplicationFont(fontPath);
    }
    
    
    qDebug() << movieBackground;
    qDebug() <<  poster_standard_width;
    qDebug() <<  poster_standard_height;
    qDebug() <<  poster_standard_border;
    qDebug() <<  poster_standard_mask;
    qDebug() << this->poster_standard_frame;
    
    return true;
}
