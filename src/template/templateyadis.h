#ifndef TEMPLATEYADIS_H
#define TEMPLATEYADIS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPixmap>

class QDomElement;
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


enum class FAN_ART {
    UNKNOWN, NONE, ERROR, OK, CANCELED
};

enum class POSTER {
    UNKNOWN, NONE, ERROR, OK
};
class QNetworkAccessManager;

#include "../promise.h"

class templateYadis :public QObject
{
    Q_OBJECT

public:
    templateYadis();
    bool loadTemplate(const QString& fileName);

public slots:

    void createTivx(QNetworkAccessManager & manager, const ScraperResource &poster, const ScraperResource &backdrop, QMap<QString, QString> texts);

private:
    QPixmap m_fanArtPixmap;
QMap<QString, QString> texts;

    FAN_ART fa=FAN_ART::UNKNOWN;
    POSTER po=POSTER::NONE;

    void setFanArt(FAN_ART fa){
        if (this->fa != fa){
        this->fa=fa;
        update();
        }
    }
    void setPoster(POSTER po){
        if (this->po != po){
        this->po=po;
        update();
        }
     }
    void update();

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

Q_SIGNALS:
    void tivxOk(QPixmap result);
    void canceled();
};

#endif // TEMPLATEYADIS_H
