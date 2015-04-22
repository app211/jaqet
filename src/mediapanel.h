#ifndef MEDIAPANEL_H
#define MEDIAPANEL_H

#include <QDialog>

class QGraphicsScene;

namespace Ui {
class MediaPanel;
}

class MediaPanel : public QDialog
{
    Q_OBJECT

public:
    explicit MediaPanel(QWidget *parent = 0);
    ~MediaPanel();
void setScene(QGraphicsScene* scene);

private:
    Ui::MediaPanel *ui;
};

#endif // MEDIAPANEL_H
