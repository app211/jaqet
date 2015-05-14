#include "inprogressdialog.h"
#include "ui_inprogressdialog.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QMovie>

InProgressDialog::InProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InProgressDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog /*| Qt::FramelessWindowHint*/);
    this->setWindowModality(Qt::ApplicationModal);

    QMovie *movie = new QMovie(":/resources/animations/ajax-loader.gif");
    ui->labelMovie->setMovie(movie);
    movie->start();

    const QRect screen = QApplication::desktop()->screenGeometry();
    this->move( screen.center() - this->rect().center() );
}

InProgressDialog::~InProgressDialog()
{
    delete ui;
}

InProgressDialog* InProgressDialog::create(){
     InProgressDialog* p=new InProgressDialog(QApplication::activeWindow());
     p->show();
     return p;
 }

