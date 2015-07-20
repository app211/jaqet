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
    QPixmap internalCreate(const CurrentItemData &data);
    void proceed(const CurrentItemData& data);

    QSize getBackdropSize() const;
    QSize getPosterSize() const;
    QSize getBannerSize() const;

    enum class Context {movie_synopsis,tv_synopsis_common,tv_synopsis_episode,tv_synopsis_episode_episodeicon};

private:
    void searchSizeForTag(QDomElement docElem, const QString& tagName, QSize& size) const;

    QString getAbsoluteFilePath(const QString& fileName);

    QPixmap buildPoster(const QPixmap& poster, const QSize& desiredSize, int standard_width, int standard_height, int standard_border, const QString& poster_standard_mask, const QString& poster_standard_frame  , const QRect &poster_inner);
    QPixmap createBackdrop(const CurrentItemData &data);


    bool exec(QPainter &result,const CurrentItemData &data);
    bool execTV(QDomElement e,QPainter &result,const CurrentItemData &data);
    bool execMovie(const QDomElement& e, QPainter &result,const CurrentItemData &data);
    bool execImage(const QDomElement& imageElement, QPainter &result,const CurrentItemData &data);
    bool execNode(QDomElement e, QPainter &result, Context context,const CurrentItemData &data);
    bool execText(const QDomElement& textElement, QPainter &pixPaint, Context context,const CurrentItemData &data);
    bool execLanguages(const QDomElement& languagesElement, QPainter &pixPaint, Context context,const CurrentItemData &data);
    bool execPoster(const QDomElement& e,const CurrentItemData &data);
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
    QRect poster_inner;

    int bannerWidth=0;
    int bannerHeight=0;
    int bannerBorder=0;
    QString bannerMask;
    QString bannerFrame;

    int episodeWidth=0;
    int episodeHeight=0;
    QString episodeMask;
    QString episodeFrame;
};

#endif // TEMPLATEYADIS_H
