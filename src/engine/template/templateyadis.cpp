#include "templateyadis.h"

#include <QXmlStreamReader>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QInputDialog>
#include <QStandardPaths>
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

bool TemplateYadis::execPoster(const QDomElement& e){
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

    return true;
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


QSize TemplateYadis::getSize(){
    int w=1920;
    int h=1080;
    return QSize(w,h);
}

QPixmap TemplateYadis::buildPoster(const QPixmap& poster, const QSize& desiredSize, int standard_width, int standard_height, int standard_border, const QString& poster_standard_mask, const QString& poster_standard_frame  ){

    QSize standardSize(standard_width,standard_height);

    QSize size= standardSize.isNull()?desiredSize:standardSize;

    QPixmap scaledPoster = poster.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QPixmap result(size);
    result.fill(Qt::transparent);
    QPainter pixPaint(&result);
    
    if (!poster_standard_mask.isEmpty()){
        QPixmap mask;
        if (!mask.load(getAbsoluteFilePath(poster_standard_mask))){
            return QPixmap();
        }

        scaledPoster.setMask(mask.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation).mask());
    }

    pixPaint.drawPixmap(0,0,size.width(),size.height(),scaledPoster);

    if (!poster_standard_frame.isEmpty()){
        QPixmap frame;
        if (!frame.load(getAbsoluteFilePath(poster_standard_frame))){
            return QPixmap();;
        }

        pixPaint.drawPixmap(0,0,size.width(),size.height(),frame.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    }

    return result;
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

QPixmap TemplateYadis::createBackdrop(){
    QPixmap result(getSize());
    result.fill(Qt::transparent);
    QPainter pixPaint(&result);

    exec(pixPaint);

    return result;
}

bool TemplateYadis::loadTemplate(const QString& fileName){

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

    QDir dir(absoluteTemplateFilePath);
    QStringList filters;
    filters << "*.ttf";

    foreach ( QString fontName, dir.entryList(filters, QDir::Files) ){
        QString fontPath =getAbsoluteFilePath(fontName);
        QFontDatabase::addApplicationFont(fontPath);
    }

    return true;
}

int TemplateYadis::getX( int x ){
    if (areas.size()>0){
        return x+areas.last().x;
    }

    return x;
}

int TemplateYadis::getY( int y ){
    if (areas.size()>0){
        return y+areas.last().y;
    }

    return y;
}

void TemplateYadis::searchSizeForTag(QDomElement docElem, const QString& tagName, QSize& size) const{
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="image" ){
                if (e.hasAttribute("type") && e.attribute("type")==tagName){
                    bool bOk;
                    int w=e.attribute("width").toInt(&bOk);
                    if (bOk){
                        int h=e.attribute("height").toInt(&bOk);
                        if (bOk){
                            size.setWidth(qMax<int>(w,size.width()));
                            size.setHeight(qMax<int>(h,size.height()));
                        }
                    }
                }
            } else {
                searchSizeForTag(e,tagName,size);
            }
        }
        n = n.nextSibling();
    }
}

QSize TemplateYadis::getPosterSize() const{
    QSize result(0,0);
    searchSizeForTag(doc.documentElement(),"poster",result);
    return result;
}

QSize TemplateYadis::getBackdropSize() const{
    QSize result(0,0);
    searchSizeForTag(doc.documentElement(),"fanart",result);
    return result;
}

QSize TemplateYadis::getBannerSize() const{
    QSize result(0,0);
    searchSizeForTag(doc.documentElement(),"banner",result);
    return result;
}

bool TemplateYadis::execText(const QDomElement& textElement, QPainter &pixPaint, Context context){
    if (!textElement.hasAttribute("type")){
        return false;
    }

    MediaInfo mediaInfo=getProperty<MediaInfo>(properties,Properties::mediainfo);

    QString type = textElement.attribute("type");
    QString color = textElement.attribute("color");
    QString language = textElement.attribute("language");
    QString font = textElement.attribute("font");

    bool bOk;
    int size=-1;
    if (textElement.hasAttribute("size")){

        size=textElement.attribute("size").toInt(&bOk);
        if (!bOk){
            return false;
        }
    }



    QString align;
    if (textElement.hasAttribute("align")){
        align=textElement.attribute("align");
    }

    int x=textElement.attribute("x").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int y=textElement.attribute("y").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int w=textElement.attribute("width").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int h=textElement.attribute("height").toInt(&bOk);
    if (!bOk){
        return false;
    }

    QString value=textElement.text();

    QString textToDraw;
    if (type=="static" && !value.isEmpty() && (language.isEmpty() ||language=="fr")){
        textToDraw=value;
    } else if (type=="plot" && properties.contains(Template::Properties::synopsis)){
        textToDraw=properties[Template::Properties::synopsis].toString();
    } else if (type=="cast" && properties.contains(Template::Properties::actors)){
        textToDraw=properties[Template::Properties::actors].toStringList().join(", ");
    } else if (type=="director" && properties.contains(Template::Properties::director)){
        textToDraw=properties[Template::Properties::director].toStringList().join(", ");
    } else if (type=="year" && properties.contains(Template::Properties::year)){
        textToDraw=properties[Template::Properties::year].toString();
    } else if (type=="runtime") {
        if (properties.contains(Template::Properties::runtime)){
            textToDraw=QDateTime::fromTime_t(properties[Template::Properties::runtime].toInt()).toUTC().toString("h'H 'mm");
        } else if (!mediaInfo.isEmpty() && mediaInfo.durationSecs()>0 ){
            textToDraw=QDateTime::fromTime_t(mediaInfo.durationSecs()).toUTC().toString("h'H 'mm");
        }
    }else if (type=="rating" && properties.contains(Template::Properties::rating)){
        textToDraw=properties[Template::Properties::rating].toString();
    }else if (type=="aired" && properties.contains(Template::Properties::aired)){
        QDateTime airedDateTime=properties[Template::Properties::aired].toDateTime();
        if (airedDateTime.isValid()){
            textToDraw=airedDateTime.toUTC().toString("dd-MM-yyyy");
        }
    }else if (type=="title") {

        if ((context==Context::tv_synopsis_common || context==Context::movie_synopsis) && properties.contains(Template::Properties::title)){
            textToDraw=properties[Template::Properties::title].toString();
        }
        else if (context==Context::tv_synopsis_episode_episodeicon && properties.contains(Template::Properties::episodetitle)){
            textToDraw=properties[Template::Properties::episodetitle].toString();
        }

        if (textElement.attribute("episodenumber")=="yes" ){
            QString separator= textElement.hasAttribute("separator")? textElement.attribute("separator") : " ";
            textToDraw = properties[Template::Properties::episode].toString()+separator+textToDraw;
         }
    } else if (type=="resolution" && !mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).isNull()){
        QSize v=mediaInfo.videoStreamValue(0, MediaInfo::VideoResolution).toSize();
        textToDraw=QString("%1").arg(v.height());
    } else if (type=="season" && properties.contains(Template::Properties::season)){
        textToDraw=properties[Template::Properties::season].toString();
    } else if (type=="channels" && !mediaInfo.audioStreamValue(0, MediaInfo::AudioChannelCount).isNull()){
        textToDraw=QString("%1").arg(mediaInfo.audioStreamValue(0, MediaInfo::AudioChannelCount).toInt());
    } else if (type=="aspect" && !mediaInfo.videoStreamValue(0, MediaInfo::VideoAspectRatioString).isNull()){
        textToDraw=QString("%1").arg(mediaInfo.videoStreamValue(0, MediaInfo::VideoAspectRatioString).toString());
    } else if (type=="network" && properties.contains(Template::Properties::network)){
        textToDraw=properties[Template::Properties::network].toString();
    }

 //   qDebug() << type << textToDraw;

    if (!textToDraw.isEmpty()){
        textToDraw = textToDraw.trimmed();

        QFont _font(font);
        if (size>0){
            _font.setPointSize(size);
        }
        pixPaint.setFont(_font);
        pixPaint.setPen(QPen(QColor(color)));

        int _align= Qt::AlignLeft;
        if (!align.isEmpty()){
            if (align.compare("center",Qt::CaseInsensitive)==0){
                _align = Qt::AlignHCenter;
            }
        }

        pixPaint.drawText(getX(x),getY(y),w,h,_align|Qt::TextWordWrap|Qt::AlignTop,textToDraw);
        /*
        const QPointF points[4] = {
            QPointF(getX(x) ,getY(y)),
            QPointF(getX(x+w),getY( y)),
            QPointF(getX(x+w), getY(y+h)),
            QPointF(getX(x),getY( y+h))
        };

        pixPaint.setPen(QPen(Qt::red));
        pixPaint.drawPolygon(points,4);
*/
    }

    return true;
}

bool TemplateYadis::execImage(const QDomElement& imageElement, QPainter &pixPaint){

    MediaInfo mediaInfo=getProperty<MediaInfo>(properties,Properties::mediainfo);

    if (!imageElement.hasAttribute("type")){
        return false;
    }

    QString type = imageElement.attribute("type");

    if (!imageElement.hasAttribute("x")){
        return false;
    }

    bool bOk;
    int x=imageElement.attribute("x").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int y=imageElement.attribute("y").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int w=imageElement.attribute("width").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int h=imageElement.attribute("height").toInt(&bOk);
    if (!bOk){
        return false;
    }

    x=getX(x);
    y=getY(y);

    QString value=imageElement.text();

    if (type=="fanart" ){
        if(properties.contains(Template::Properties::backdrop)){
            QPixmap backdrop=properties[Template::Properties::backdrop].value<QPixmap>();
            if (!backdrop.isNull()){
                QPixmap scaled=backdrop.scaled(QSize(w,h),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
                int _x= x+ (w-scaled.width())/2;
                int _y= y+ (h-scaled.height())/2;
                int _w= scaled.width();
                int _h=scaled.height();
                pixPaint.drawPixmap(_x,_y,_w,_h,scaled);
            }
        }
    } else if (type=="poster"){
        if(properties.contains(Template::Properties::poster)){
            QPixmap poster=properties[Template::Properties::poster].value<QPixmap>();
            if (!poster.isNull()){
                QPixmap scaled= buildPoster(poster,QSize(w,h),  poster_standard_width.toInt(), poster_standard_height.toInt(), poster_standard_border.toInt(), poster_standard_mask,  poster_standard_frame  );
                qDebug() << QSize(w,h) << scaled.size();
                int _x= x+ (w-scaled.width())/2;
                int _y= y+ (h-scaled.height())/2;
                int _w= scaled.width();
                int _h=scaled.height();
                pixPaint.drawPixmap(_x,_y,_w,_h,scaled);
            }
        }
    }else if (type=="thumbnail"){
        if(properties.contains(Template::Properties::thumbnail)){
            QPixmap thumbnail=properties[Template::Properties::thumbnail].value<QPixmap>();
            if (!thumbnail.isNull()){
                QPixmap scaled= buildPoster(thumbnail,QSize(w,h),  episodeWidth, episodeHeight, 0, episodeMask,  episodeFrame  );
                int _x= x+ (w-scaled.width())/2;
                int _y= y+ (h-scaled.height())/2;
                int _w= scaled.width();
                int _h=scaled.height();
                pixPaint.drawPixmap(_x,_y,_w,_h,scaled);
            }
        }
    } else if (type=="banner"){
        if(properties.contains(Template::Properties::banner)){
            QPixmap banner=properties[Template::Properties::banner].value<QPixmap>();
            if (!banner.isNull()){
                QPixmap scaled= buildPoster(banner,QSize(w,h),  bannerWidth, bannerHeight, bannerBorder, bannerMask,  bannerFrame  );
                int _x= x+ (w-scaled.width())/2;
                int _y= y+ (h-scaled.height())/2;
                int _w= scaled.width();
                int _h=scaled.height();
                pixPaint.drawPixmap(_x,_y,_w,_h,scaled);
            }
        }
    } else if (type=="static" && !value.isEmpty()){
        QPixmap pstatic;

        if (pstatic.load(getAbsoluteFilePath(value)) ){
            pixPaint.drawPixmap(x,y,w,h,pstatic);
        }
    } else if (type=="vcodec"){
        if (mediaInfo.videoStreamCount()>0){
            QPixmap pstatic;
            QString f=QString("%1.png").arg(mediaInfo.videoStreamValue(0, MediaInfo::VideoCodec).toString());
            if (pstatic.load(getAbsoluteFilePath(f)) ){
                pixPaint.drawPixmap(x,y,w,h,pstatic);
            }
        }
    }else if (type=="acodec"){
        if (mediaInfo.audioStreamCount()>0){
            QPixmap pstatic;
            QString acodec=mediaInfo.audioStreamValue(0, MediaInfo::AudioCodec).toString().trimmed();

            QString f=QString("%1.png").arg(acodec.split(" ").at(0));
            if (pstatic.load(getAbsoluteFilePath(f)) ){
                pixPaint.drawPixmap(x,y,w,h,pstatic);
            }
        }
    }  else if (type=="format"){
        if (mediaInfo.audioStreamCount()>0){
            QPixmap pstatic;
            QString f=QString("%1.png").arg(mediaInfo.metaDataValue( MediaInfo::Format).toString());
            if (pstatic.load(getAbsoluteFilePath(f)) ){
                pixPaint.drawPixmap(x,y,w,h,pstatic);
            }
        }
    }else if (type=="aspect"){
        if (mediaInfo.videoStreamCount()>0 && !mediaInfo.videoStreamValue(0, MediaInfo::VideoAspectRatioString).isNull()){
            QString aspectRatioString=mediaInfo.videoStreamValue(0, MediaInfo::VideoAspectRatioString).toString();
            QStringList ratio=aspectRatioString.split(QRegExp("(/|:)"));
            QString f;
            QPixmap pstatic;
            if (ratio.size()==2){
                if (ratio.at(1)=="1"){
                    f=QString("%1.png").arg(ratio.at(0));
                } else {
                    f=QString("%1_%2.png").arg(ratio.at(0),ratio.at(1));
                }
                if (pstatic.load(getAbsoluteFilePath(f)) ){
                    pixPaint.drawPixmap(x,y,w,h,pstatic);
                }
            }
        }
    }


    return true;
}


bool TemplateYadis::execLanguages(const QDomElement& languagesElement, QPainter &pixPaint, Context context){

    MediaInfo mediaInfo=getProperty<MediaInfo>(properties,Properties::mediainfo);

    if (mediaInfo.audioStreamCount()==0){
        return true;
    }

    QStringList audioLanguages;
    for (int i=0; i<mediaInfo.audioStreamCount();i++){
        QString audioLanguage=mediaInfo.audioStreamValue(i, MediaInfo::AudioLanguage).toString();
        if (!audioLanguage.isEmpty()){
            audioLanguages << audioLanguage;
        }
    }

    QDomElement  audioElement = languagesElement.firstChildElement("audio");
    if (audioElement.isNull()){
        return false;
    }

    QDomElement  backimageElement = languagesElement.firstChildElement("backimage");
    if (backimageElement.isNull()){
        return false;
    }

    QDomElement  textElement = languagesElement.firstChildElement("text");
    if (textElement.isNull()){
        return false;
    }

    bool bOk;
    int x_audio=audioElement.attribute("x").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int y_audio=audioElement.attribute("y").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int width=backimageElement.attribute("width").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int height=backimageElement.attribute("height").toInt(&bOk);
    if (!bOk){
        return false;
    }

    int spacing=backimageElement.attribute("spacing").toInt(&bOk);
    if (!bOk){
        return false;
    }

    QString font=textElement.attribute("font");

    int size=textElement.attribute("size").toInt(&bOk);
    if (!bOk){
        return false;
    }

    QString color=textElement.attribute("color");

    int x= x_audio;
    int y=y_audio;
    int w= width;
    int h=height;

    QString image = backimageElement.text();

    for (int i=0; i<qMin<int>(3,audioLanguages.size());i++){
        QString textToDraw=audioLanguages.at(i).toUpper();

        if (!textToDraw.isEmpty()){
            if (!font.isEmpty()){
                QFont _font(font);
                if (size>0){
                    _font.setPointSize(size);
                }
                pixPaint.setFont(_font);
            }

            if (!color.isEmpty()){
                pixPaint.setPen(QPen(QColor(color)));
            }

            if (!image.isEmpty()){
                QPixmap pstatic;

                if (pstatic.load(getAbsoluteFilePath(image)) ){
                    pixPaint.drawPixmap(getX(x) ,getY(y),w,h,pstatic);
                }
            }

            pixPaint.drawText(getX(x),getY(y),w,h,Qt::AlignCenter,textToDraw);

            x += spacing;
        }
    }

    return true;
}

bool TemplateYadis::execNode(QDomElement synopsisNode, QPainter &result, Context context){
    if (synopsisNode.isNull()){
        return false;
    }

    int nbAreas=areas.size();

    QDomNode n = synopsisNode.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="languages"){
                execLanguages(e,result,context);
            } else if (e.tagName()=="image"){
                execImage(e,result);
            }else if (e.tagName()=="text"){
                execText(e,result,context);
            }else if (e.tagName()=="area"){
                templateYadis_area area;
                bool bOk;
                area.x=e.attribute("x").toInt(&bOk);
                if (!bOk){
                    return false;
                }

                area.y=e.attribute("y").toInt(&bOk);
                if (!bOk){
                    return false;
                }

                area.w=e.attribute("width").toInt(&bOk);
                if (!bOk){
                    return false;
                }

                area.h=e.attribute("height").toInt(&bOk);
                if (!bOk){
                    return false;
                }

                areas.append(area);
            }
        }

        n = n.nextSibling();
    }

    int nbToRemove=areas.size()-nbAreas;
    while (nbToRemove>0){
        nbToRemove--;
        areas.removeLast();
    }
}

bool TemplateYadis::execMovie(const QDomElement& e, QPainter &result){

    if (e.hasAttribute("background")){
        movieBackground = e.attribute("background");
    }

    QDomNode n = e.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="poster"){
                execPoster(e);
            }else if (e.tagName()=="synopsis"){
                execNode(e,result,Context::movie_synopsis);
            }
        }

        n = n.nextSibling();
    }
}

bool TemplateYadis::execTV(QDomElement e, QPainter &result){

    if (e.hasAttribute("background")){
        tvBackground = e.attribute("background");
    }


    QDomNode n = e.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName() == "banner"){
                QDomNode n2 = e.firstChild();
                while(!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    if(!e2.isNull()) {
                        if (e2.tagName()=="size"){
                            bannerWidth=e2.attribute("width","0").toInt();
                            bannerHeight=e2.attribute("height","0").toInt();
                            bannerBorder=e2.attribute("border","0").toInt();

                        } else if (e2.tagName()=="mask"){
                            bannerMask=e2.text();
                        }else if (e2.tagName()=="frame"){
                            bannerFrame=e2.text();
                        }
                    }
                    n2 = n2.nextSibling();
                }
            } else if (e.tagName() == "episode"){
                QDomNode n2 = e.firstChild();
                while(!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    if(!e2.isNull()) {
                        if (e2.tagName()=="size"){
                            episodeWidth=e2.attribute("width","0").toInt();
                            episodeHeight=e2.attribute("height","0").toInt();

                        } else if (e2.tagName()=="mask"){
                            episodeMask=e2.text();
                        }else if (e2.tagName()=="frame"){
                            episodeFrame=e2.text();
                        }
                    }
                    n2 = n2.nextSibling();
                }
            } else if (e.tagName()=="poster"){
                // parseMoviePoster(e);
            }else if (e.tagName()=="synopsis"){
                QDomNode n2 = e.firstChild();
                while(!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    if(!e2.isNull()) {
                        if (e2.tagName()=="common"){
                            execNode(e2,result,Context::tv_synopsis_common);
                        } else if (e2.tagName()=="season"){
                            // Season is ignored
                        }else if (e2.tagName()=="episode"){
                            execNode(e2,result,Context::tv_synopsis_episode);
                            QDomNodeList episodeIcons=e2.elementsByTagName("episodeicon");
                            if (episodeIcons.length()==1){
                                execNode(episodeIcons.at(0).toElement(),result,Context::tv_synopsis_episode_episodeicon);
                            }
                        }
                    }
                    n2 = n2.nextSibling();
                }
            }
        }

        n = n.nextSibling();
    }
}

bool TemplateYadis::exec(QPainter &pixPainter){

    bool tv=properties[Template::Properties::tv].toBool();

    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if (e.tagName()=="movie" && !tv){
                execMovie(e,pixPainter);
            } else if (e.tagName()=="tv" && tv){
                execTV(e,pixPainter);
            }
        }
        n = n.nextSibling();
    }


    return true; // result;

}
