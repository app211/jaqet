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

private:
    Ui::MediaChooserPopup *ui;

    void setImageFromInternet(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate, int x, int y, int w, int h);
    void addFile(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator , QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton,  int x, int y, int w, int h);
    void addError(const QString& errorMessage,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton, QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton, int x, int y, int w, int h);
    void addHttpRequest(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, int x, int y, int w, int h,  QPointer<QGraphicsProxyWidget> busyIndicator, QPointer<ClickButtonMediaChooserGraphicsObject> chooseButton,QPointer<ClickButtonMediaChooserGraphicsObject> reloadButton );
    void addImage(const QUrl&, const MediaChoosed &mediaChoosed, QFlags<ImageType> type);

    void startPromise( QNetworkAccessManager* manager);


    QFlags<ImageType> _currentFilter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaChooserPopup::imageFilter)

#endif // MEDIACHOOSERWIDGET_H
