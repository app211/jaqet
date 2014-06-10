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
