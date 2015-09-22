#ifndef MEDIACHOOSERWIDGET_H
#define MEDIACHOOSERWIDGET_H

#include <QWidget>
#include <QFlags>
#include <QUrl>

#include "mediachoosermediatype.h"

class QGraphicsScene;
class QCloseEvent;
class MediaChooserGraphicsObject;
class MediaChooserButton;
class Scraper;
class QNetworkAccessManager;
class ClickButtonMediaChooserGraphicsObject;

namespace Ui {
class MediaChooserPopup;
}


class MediaChooserPopup: public QWidget
{
    Q_OBJECT

public:
    Q_DECLARE_FLAGS(imageFilter, ImageType)

    explicit MediaChooserPopup(QWidget *parent = 0);
    ~MediaChooserPopup();
    void clear();
    void addImageFromUrl(const QUrl& url, QFlags<ImageType> type);
    void addImageFromFile(const QString& localFile, QFlags<ImageType> type);
    void addImageFromScraper(const Scraper* scraper, const QString& imageRef , const QSize &originalSize, QFlags<ImageType> type);

    void popup(MediaChooserButton *button, QFlags<ImageType> filter=ImageType::All);

    QFlags<ImageType> currentFilter();

    uint getMediaWidth() const {
        return 200;
    }

    uint getMediaHeigth() const {
        return 200;
    }

protected:
     void closeEvent(QCloseEvent *event);
     void doFilter( QFlags<ImageType> filter);
signals:
     void mediaChoosed(const MediaChoosed& url);

     void popupClosed();

private slots:
      void refilter();


      void on_pushButton_clicked();

public slots:
      void mediaSelected(const MediaChoosed& url);
      void reload(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton);

private:
    Ui::MediaChooserPopup *ui;

    void setImageFromInternet(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate);
    void addFile(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator , QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton);
    void addError(const QString& errorMessage,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton);
    void addHttpRequest(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton );
    void addImage(const QUrl&, const MediaChoosed &mediaChoosed, QFlags<ImageType> type);
    void addPixmap(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate,QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton);

    void startPromise( QNetworkAccessManager* manager);

    QPixmap createDefaultPoster() const;


    QFlags<ImageType> _currentFilter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaChooserPopup::imageFilter)

#endif // MEDIACHOOSERWIDGET_H
