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
    void setScene(QGraphicsScene* scene);
    void addImageFormUrl(const QUrl&, QFlags<ImageType> type);
    void addImageFromFile(const QString& localFile, QFlags<ImageType> type);
    void popup(MediaChooserButton *button, QFlags<ImageType> filter=ImageType::All);

    QFlags<ImageType> currentFilter();
protected:
     void closeEvent(QCloseEvent *event);
     void doFilter( QFlags<ImageType> filter);
signals:
     void mediaSelected(const QUrl& url);
     void popupClosed();

private slots:
      void refilter();


      void on_pushButton_clicked();

private:
    Ui::MediaChooserPopup *ui;

    void setImageFromInternet(const QPixmap& pixmap,  QPointer<MediaChooserGraphicsObject> itemToUpdate, int x, int y, int w, int h);
    void addFile(const QUrl& url,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator ,  int x, int y, int w, int h);
    void addError(const QString& errorMessage,  QPointer<MediaChooserGraphicsObject> itemToUpdate, QPointer<QGraphicsProxyWidget> busyIndicator, int x, int y, int w, int h);

    QFlags<ImageType> _currentFilter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaChooserPopup::imageFilter)

#endif // MEDIACHOOSERWIDGET_H
