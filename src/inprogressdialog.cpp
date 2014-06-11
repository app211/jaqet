#include "inprogressdialog.h"
#include "ui_inprogressdialog.h"

InProgressDialog::InProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InProgressDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
}

InProgressDialog::~InProgressDialog()
{
    delete ui;
}


 InProgressDialog* InProgressDialog::create(){
     InProgressDialog* p=new InProgressDialog(QApplication::activeWindow());
     p->show();
     QApplication::processEvents();

     return p;
 }
