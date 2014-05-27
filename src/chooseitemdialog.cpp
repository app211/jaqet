#include "chooseitemdialog.h"
#include "ui_chooseitemdialog.h"

ChooseItemDialog::ChooseItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseItemDialog)
{
    ui->setupUi(this);
}

ChooseItemDialog::~ChooseItemDialog()
{
    delete ui;
}

void ChooseItemDialog::setList(ShowPtrList shows){
    ui->listItem->clear();
    this->shows=shows;
    for (int i=0; i<shows.size();i++){
       QListWidgetItem *newItem = new QListWidgetItem;
       newItem->setText(shows.at(i)->title+" "+ shows.at(i)->productionYear);
       newItem->setData(Qt::UserRole,i);
       ui->listItem->addItem(newItem);
    }
}

void ChooseItemDialog::setList(FilmPrtList result){
    ui->listItem->clear();
    this->films=result;
    for (int i=0; i<result.size();i++){
       QListWidgetItem *newItem = new QListWidgetItem;
       newItem->setText(result.at(i)->title+" "+ result.at(i)->productionYear);
       newItem->setData(Qt::UserRole,i);
       ui->listItem->addItem(newItem);
    }
}

FilmPtr ChooseItemDialog::getSelectedFilm() const{
     QList<QListWidgetItem *> selectedItems=ui->listItem->selectedItems();
     if (selectedItems.size()==1){
         return films.at(selectedItems.at(0)->data(Qt::UserRole).toInt());
     }


     return FilmPtr();
}

ShowPtr ChooseItemDialog::getSelectedShow() const{
     QList<QListWidgetItem *> selectedItems=ui->listItem->selectedItems();
     if (selectedItems.size()==1){
         return shows.at(selectedItems.at(0)->data(Qt::UserRole).toInt());
     }


     return ShowPtr();
}
