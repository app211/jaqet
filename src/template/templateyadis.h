#ifndef TEMPLATEYADIS_H
#define TEMPLATEYADIS_H

#include <QString>
#include <QList>

class QDomElement;
class QPixmap;
class templateYadis;
class ScraperResource;

class  templateYadis_text {
    QString type;
    QString color;
    QString language;
    QString font;
    QString value;
    int x;
    int y;
    int w;
    int h;
    int size;

     friend templateYadis;
};

class  templateYadis_image  {
    QString type;
    int x;
    int y;
    int w;
    int h;
    QString value;

     friend templateYadis;
};
class templateYadis
{
public:
    templateYadis();
    bool loadTemplate(const QString& fileName);
     QPixmap createTivx(const ScraperResource &poster, const ScraperResource &backdrop, const QString& synopsis);
  private:
    void parseMovie(const QDomElement& e);
    void parseMoviePoster(const QDomElement& e);
    QString absoluteTemplateFilePath;
    bool parseImage(const QDomElement& e, templateYadis_image& image);

    bool parseText(const QDomElement& textElement, templateYadis_text& text);
    QString getAbsoluteFilePath(const QString& fileName);
void parseSynopsis(const QDomElement& synopsisNode);
bool buildPoster(const ScraperResource& poster, QPixmap &pixmap);

public:
     QString movieBackground;
     QString poster_standard_width;
     QString poster_standard_height;
     QString poster_standard_border;
 QString poster_standard_mask;
 QString poster_standard_frame;
 QList<templateYadis_image> movie_synopsis_images;
 QList<templateYadis_text> movie_synopsis_texts;

};

#endif // TEMPLATEYADIS_H
