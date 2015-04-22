#include "mediapanel.h"
#include "ui_mediapanel.h"

#include <QGraphicsScene>

MediaPanel::MediaPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MediaPanel)
{

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->setupUi(this);
}

MediaPanel::~MediaPanel()
{
    delete ui;
}

void MediaPanel::setScene(QGraphicsScene* scene){
    ui->graphicsView->setScene(scene);
}
