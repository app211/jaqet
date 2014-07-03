#ifndef TEMPLATEYADIS_H
#define TEMPLATEYADIS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QDomElement>

#include "template.h"
#include "../../scanner/scanner.h"

class TemplateYadis;
class ScraperResource;

class  templateYadis_area  {
    int x=-1;
    int y=-1;
    int w=-1;
    int h=-1;

    bool isNull() const {
        return x==-1 || y==-1 || w==-1 || h==-1;
    }

    friend TemplateYadis;
};

class TemplateYadis :public Template
{
    Q_OBJECT
public:
    TemplateYadis();
    bool loadTemplate(const QString& fileName);
    QSize getSize();
    void internalCreate();
    void proceed();

    QSize getBackdropSize() const;
    QSize getPosterSize() const;
    QSize getBannerSize() const;

    enum class Context {movie_synopsis,tv_synopsis_common,tv_synopsis_episode,tv_synopsis_episode_episodeicon};

private:
    QMap<QString, QString> texts;

    void parseMoviePoster(const QDomElement& e);
void searchSizeForTag(QDomElement docElem, const QString& tagName, QSize& size) const;

    QString getAbsoluteFilePath(const QString& fileName);

    bool buildPoster(const ScraperResource& poster, QPixmap &pixmap);
    void  proceed(const QFileInfo& f);
    QPixmap createBackdrop();


    bool exec(QPainter &result);
    bool execTV(QDomElement e,QPainter &result);
    bool execMovie(const QDomElement& e, QPainter &result);
    bool execImage(const QDomElement& imageElement, QPainter &result);
    bool execNode(QDomElement e, QPainter &result, Context context);
    bool execText(const QDomElement& textElement, QPainter &pixPaint, Context context);
    int getX( int x );
    int getY( int y );

    QList<templateYadis_area> areas;
    QDomDocument doc;
    QString absoluteTemplateFilePath;

public:
    QString movieBackground;
    QString tvBackground;
    QString poster_standard_width;
    QString poster_standard_height;
    QString poster_standard_border;
    QString poster_standard_mask;
    QString poster_standard_frame;


signals :
    void tivxOk(QPixmap result);
    void canceled();
};

#endif // TEMPLATEYADIS_H
