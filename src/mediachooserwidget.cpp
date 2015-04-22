#include "mediachooserwidget.h"
#include "ui_mediachooserwidget.h"

MediaChooserWidget::MediaChooserWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaChooserWidget)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    ui->setupUi(this);
}

MediaChooserWidget::~MediaChooserWidget()
{
    delete ui;
}

void MediaChooserWidget::setScene(QGraphicsScene* scene){
    ui->graphicsView->setScene(scene);
}

void MediaChooserWidget::focusOutEvent ( QFocusEvent * event ) {
    close();
}
