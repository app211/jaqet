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
#include <QNetworkReply>

#include "../../scrapers/scraper.h"

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
                parseSynopsis(e, movie_synopsis_texts, movie_synopsis_images);
            }
        }
        
        
        n = n.nextSibling();
    }
}


void TemplateYadis::parseTV(const QDomElement& e){

    if (e.hasAttribute("background")){
        tvBackground = e.attribute("background");
    }

    QDomNode n = e.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="poster"){
                parseMoviePoster(e);
            }else if (e.tagName()=="synopsis"){
                QDomNode n2 = e.firstChild();
                while(!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    if(!e2.isNull()) {
                        if (e2.tagName()=="common"){
                            parseSynopsis(e2, commontv_synopsis_texts, commontv_synopsis_images);

                        } else if (e2.tagName()=="season"){
                            parseSynopsis(e2, seasontv_synopsis_texts, seasontv_synopsis_images);

                        }else if (e2.tagName()=="episode"){
                            parseSynopsis(e2, episodetv_synopsis_texts, episodetv_synopsis_images);
                        }
                    }
        n2 = n2.nextSibling();
                }
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

    if (textElement.hasAttribute("align")){
        text.align=textElement.attribute("align");
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

void TemplateYadis::parseSynopsis(const QDomElement& synopsisNode, QList<templateYadis_text>&synopsis_texts,  QList<templateYadis_image>& synopsis_images ){
    QDomNode n = synopsisNode.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="image"){
                templateYadis_image image;
                parseImage(e,image);
                synopsis_images.append(image);
            }else if (e.tagName()=="text"){
                templateYadis_text text;
                parseText(e,text);
                synopsis_texts.append(text);
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

void TemplateYadis::drawText( QPainter& pixPaint, const templateYadis_text& text, const QString& textToDraw){
    QFont font(text.font);
    font.setPointSize(text.size);
    pixPaint.setFont(font);
    pixPaint.setPen(QPen(QColor(text.color)));

    int align= Qt::AlignLeft;
    if (!text.align.isEmpty()){
        if (text.align.compare("center",Qt::CaseInsensitive)==0){
            align = Qt::AlignCenter;
        }
    }

    pixPaint.drawText(text.x,text.y,text.w,text.h,align|Qt::TextWordWrap|Qt::AlignTop	,textToDraw);

    const QPointF points[4] = {
        QPointF(text.x, text.y),
        QPointF(text.x+text.w, text.y),
        QPointF(text.x+text.w, text.y+text.h),
        QPointF(text.x, text.y+text.h)
    };

    pixPaint.setPen(QPen(Qt::red));
    pixPaint.drawPolygon(points,4);
}

template <class T> T getProperty(const QMap<Template::Properties, QVariant>& properties, Template::Properties property, const T& defaultValue=T() ){

    if (properties.contains(property) && properties[property].canConvert<T>()){
        return properties[property].value<T>();
    }

    return defaultValue;
}

void  TemplateYadis::proceed(const QFileInfo& f){
    QString title=getProperty<QString>(properties,Properties::title,"jaqet");
    QString suffixe="";
    int counter=0;
    while (QFileInfo(f.absoluteDir(),title+suffixe).exists()){
        suffixe=QString::number(counter++);
    }

    QDir d;

    if (!d.mkpath( QFileInfo(f.absoluteDir(),title+suffixe).absoluteFilePath())){
        return;
    }

    d.setPath(QFileInfo(f.absoluteDir(),title+suffixe).absoluteFilePath());

    QFile file(f.absoluteFilePath());
    if (!file.rename(QFileInfo(d,title+"."+f.completeSuffix()).absoluteFilePath())){
        return;
    }


    QPixmap back=createBackdrop();
    back.save(QFileInfo(d,"tvix.jpg").absoluteFilePath());

    QPixmap poster=getProperty<QPixmap>(properties,Properties::poster);
    if (!poster.isNull()){
        poster.save(QFileInfo(d,"folder.jpg").absoluteFilePath());
    }
}

void TemplateYadis::proceed(){
    if (properties.contains(Template::Properties::fileinfo) && properties[Template::Properties::fileinfo].canConvert<QFileInfo>()){
        QFileInfo f= properties[Template::Properties::fileinfo].value<QFileInfo>();
        if (f.exists()){
            return proceed(f);
        }
    }
}
void TemplateYadis::internalCreate(){

    QPixmap result=createBackdrop();
    emit tivxOk(result.scaled(QSize(result.width()/2,result.height()/2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));

}

void  TemplateYadis::drawTextes(QPainter& pixPaint, QList<templateYadis_text>& texts,  MediaInfo& mediaInfo){
    foreach (const templateYadis_text& text, texts){
        if (text.type=="static" && !text.value.isEmpty() && (text.language.isEmpty() ||text.language=="fr")){
            drawText(pixPaint,text,text.value);

        } else if (text.type=="plot" && properties.contains(Template::Properties::synopsis)){
            drawText(pixPaint,text,properties[Template::Properties::synopsis].toString());

        } else if (text.type=="cast" && properties.contains(Template::Properties::actors)){
            drawText(pixPaint,text,properties[Template::Properties::actors].toStringList().join(", "));

        } else if (text.type=="director" && properties.contains(Template::Properties::director)){
            drawText(pixPaint,text,properties[Template::Properties::director].toStringList().join(", "));

        } else if (text.type=="year" && properties.contains(Template::Properties::year)){
            drawText(pixPaint,text,properties[Template::Properties::year].toString());

        } else if (text.type=="runtime" && properties.contains(Template::Properties::runtime)){
            drawText(pixPaint,text, properties[Template::Properties::runtime].toString());

        }else if (text.type=="title" && properties.contains(Template::Properties::title)){
            drawText(pixPaint,text,properties[Template::Properties::title].toString());

        } else if (text.type=="resolution" && !mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).isNull()){
            QSize v=mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).toSize();
            QString s=QString("%1").arg(v.width());
            drawText(pixPaint,text,s);
        }
    }
}

void TemplateYadis::drawImages(QPainter& pixPaint, QList<templateYadis_image>& images){
    foreach (const templateYadis_image& image, images){
        if (image.type=="fanart" ){
            if(properties.contains(Template::Properties::backdrop)){
                QPixmap backdrop=properties[Template::Properties::backdrop].value<QPixmap>();
                if (!backdrop.isNull()){
                    QPixmap scaled=backdrop.scaled(QSize(image.w,image.h),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                    int x= image.x+ (image.w-scaled.width())/2;
                    int y= image.y+ (image.h-scaled.height())/2;
                    int w= scaled.width();
                    int h=scaled.height();
                    pixPaint.drawPixmap(x,y,w,h,scaled);
                }
            }
        } else if (image.type=="poster"){
            if(properties.contains(Template::Properties::poster)){
                QPixmap poster=properties[Template::Properties::poster].value<QPixmap>();
                if (!poster.isNull()){
                    QPixmap scaled=poster.scaled(QSize(image.w,image.h),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                    int x= image.x+ (image.w-scaled.width())/2;
                    int y= image.y+ (image.h-scaled.height())/2;
                    int w= scaled.width();
                    int h=scaled.height();
                    pixPaint.drawPixmap(x,y,w,h,scaled);
                }
            }
        } else if (image.type=="static" && !image.value.isEmpty()){
            QPixmap pstatic;

            if (pstatic.load(getAbsoluteFilePath(image.value)) ){
                pixPaint.drawPixmap(image.x,image.y,image.w,image.h,pstatic);
            }
        }

    }

}
QPixmap TemplateYadis::createBackdrop(){
    MediaInfo mediaInfo=getProperty<MediaInfo>(properties,Properties::mediainfo);


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

    bool tv=properties[Template::Properties::tv].toBool();

    if (tv){
        drawImages(pixPaint, commontv_synopsis_images);
        drawImages(pixPaint, seasontv_synopsis_images);
        drawImages(pixPaint, episodetv_synopsis_images);
    } else {
        drawImages(pixPaint, movie_synopsis_images);
    }

    if (tv){
        drawTextes(pixPaint, commontv_synopsis_texts,mediaInfo);
        drawTextes(pixPaint, seasontv_synopsis_texts,mediaInfo);
        drawTextes(pixPaint, episodetv_synopsis_texts,mediaInfo);
    } else {
        drawTextes(pixPaint, movie_synopsis_texts,mediaInfo);
    }



    return result;
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
            } else if (e.tagName()=="tv"){
                parseTV(e);

            }

            qDebug() << e.tagName();
        }
        n = n.nextSibling();
    }

    QDir dir(absoluteTemplateFilePath);
    QStringList filters;
    filters << "*.ttf";

    foreach ( QString fontName, dir.entryList(filters, QDir::Files) ){
        QString fontPath =getAbsoluteFilePath(fontName);
        QFontDatabase::addApplicationFont(fontPath);
    }


    backdropSize = QSize(0,0);
    posterSize = QSize(0,0);

    foreach (const templateYadis_image& image, movie_synopsis_images){
        if (image.type=="poster"){
            posterSize.setWidth(qMax<int>(posterSize.width(),image.w));
            posterSize.setHeight(qMax<int>(posterSize.height(),image.h));
        } else if (image.type=="fanart" ){
            backdropSize.setWidth(qMax<int>(backdropSize.width(),image.w));
            backdropSize.setHeight(qMax<int>(backdropSize.height(),image.h));
        }
    }

    return true;
}
