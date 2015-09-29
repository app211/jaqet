#include "maskinputdialog.h"

#include<QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

MaskInputDialog::MaskInputDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent)
{
    if(flags!=0) setWindowFlags(flags);

    QVBoxLayout *l=new QVBoxLayout(this);

    label=new QLabel(this);


    text=new QLineEdit(this);
    //connect(text, SIGNAL(textChanged(QString)), this, SLOT(checkValid(QString)));

    buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    l->addWidget(label);
    l->addWidget(text);
    l->addWidget(buttonBox);

}

void MaskInputDialog::setTitle(const QString &title){ setWindowTitle(title); }
void MaskInputDialog::setLabelText(const QString &label){ this->label->setText(label); }
void MaskInputDialog::setText(const QString &text){ this->text->setText(text); }
void MaskInputDialog::setInputMask(const QString &inputMask){ this->text->setInputMask(inputMask); }


QString MaskInputDialog::getLabelText(){ return label->text(); }
QString MaskInputDialog::getText(){ return text->text(); }


QString MaskInputDialog::getText(QWidget *parent, const QString &title, const QString &label, const QString &text, const QString &inputMask, bool *ok, Qt::WindowFlags flags){
    MaskInputDialog *r=new MaskInputDialog(parent, flags);
    r->setTitle(title);
    r->setLabelText(label);
    r->setText(text);
    r->setInputMask(inputMask);
    *ok=r->exec()==QDialog::Accepted;
    if(*ok) return r->getText();
    else return QString();
}
