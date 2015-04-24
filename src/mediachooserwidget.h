#ifndef MEDIACHOOSERWIDGET_H
#define MEDIACHOOSERWIDGET_H

#include <QWidget>
#include "scrapers/scraper.h"

class QGraphicsScene;
class QCloseEvent;

namespace Ui {
class MediaChooserWidget;
}

class MediaChooserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MediaChooserWidget(QWidget *parent = 0);
    ~MediaChooserWidget();
    void setScene(QGraphicsScene* scene);
    void popup(Scraper::ImageType filter);

protected:
     void closeEvent(QCloseEvent *event);
signals:
     void WidgetClosed();
     void myfocusOutEvent(QFocusEvent *event);

private:
    Ui::MediaChooserWidget *ui;
};

#endif // MEDIACHOOSERWIDGET_H
