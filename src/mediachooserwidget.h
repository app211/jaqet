#ifndef MEDIACHOOSERWIDGET_H
#define MEDIACHOOSERWIDGET_H

#include <QWidget>

class QGraphicsScene;

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

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    Ui::MediaChooserWidget *ui;
};

#endif // MEDIACHOOSERWIDGET_H
