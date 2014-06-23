#ifndef TEMPLATEYADIS_H
#define TEMPLATEYADIS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPixmap>

#include "template.h"
#include "../../scanner/scanner.h"

class QDomElement;
class TemplateYadis;
class ScraperResource;


class  templateYadis_text {
    QString type;
    QString color;
    QString language;
    QString font;
    QString value;
    QString align;
    int x;
    int y;
    int w;
    int h;
    int size;

    friend TemplateYadis;
};

class  templateYadis_image  {
    QString type;
    int x;
    int y;
    int w;
    int h;
    QString value;

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

private:
    QMap<QString, QString> texts;

    void parseMovie(const QDomElement& e);
    void parseTV(const QDomElement& e);
     void parseMoviePoster(const QDomElement& e);
    QString absoluteTemplateFilePath;
    bool parseImage(const QDomElement& e, templateYadis_image& image);

    bool parseText(const QDomElement& textElement, templateYadis_text& text);
    QString getAbsoluteFilePath(const QString& fileName);
    void parseSynopsis(const QDomElement& synopsisNode, QList<templateYadis_text>&synopsis_texts,  QList<templateYadis_image>& synopsis_images );

    bool buildPoster(const ScraperResource& poster, QPixmap &pixmap);
    void drawText(QPainter &pixPaint, const templateYadis_text& text, const QString& textToDraw);
    void  proceed(const QFileInfo& f);
    QPixmap createBackdrop();
void drawImages(QPainter& pixPaint, QList<templateYadis_image>& images);
void  drawTextes(QPainter& pixPaint, QList<templateYadis_text>& texts, MediaInfo &mediaInfo);

public:
    QString movieBackground;
    QString tvBackground;
    QString poster_standard_width;
    QString poster_standard_height;
    QString poster_standard_border;
    QString poster_standard_mask;
    QString poster_standard_frame;
    QList<templateYadis_image> movie_synopsis_images;
    QList<templateYadis_text> movie_synopsis_texts;
    QList<templateYadis_image> commontv_synopsis_images;
    QList<templateYadis_text> commontv_synopsis_texts;
    QList<templateYadis_image> seasontv_synopsis_images;
    QList<templateYadis_text> seasontv_synopsis_texts;
    QList<templateYadis_image> episodetv_synopsis_images;
    QList<templateYadis_text> episodetv_synopsis_texts;

signals :
    void tivxOk(QPixmap result);
    void canceled();
};

#endif // TEMPLATEYADIS_H
