#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <QTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QInputDialog>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QCoreApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QPen>
#include <QBrush>

#include "scrapers/allocinescraper.h"
#include "scrapers/themoviedbscraper.h"
#include "scrapers/thetvdbscraper.h"

#include "engine/tvixengine.h"
#include "promise.h"
#include "searchscraperdialog.h"
#include "configdialog/configdialog.h"

void MainWindow::doubleClicked ( const QModelIndex & ){

 /*  if (index.isValid()){
        QVariant v=modelB->data(index,Qt::DisplayRole);
        if (v==".."){
            modelB->cdUp();
        } else {
            modelB->cd(v.toString());

        }
    }*/
}


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    ui->setupUi(this);

    modelB=new TVIXEngine(this, QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0));

    QSortFilterProxyModel* f =new QSortFilterProxyModel(this);
    f->setSourceModel(modelB);

    ui->listView->setModel(f);
    connect(ui->listView, SIGNAL(doubleClicked ( const QModelIndex & )),
            modelB, SLOT(doubleClicked ( const QModelIndex & )));


    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));


    connect(f,SIGNAL(modelAboutToBeReset()), this, SLOT(modelAboutToBeReset()));
    connect(f,SIGNAL(modelReset()), this, SLOT(modelReset()));

    scene = new QGraphicsScene(this);
   // ui->widget_3->setScene(scene);

    //ui->scrollAreaScrapResult->setVisible(false);


}





void MainWindow::modelAboutToBeReset(){
    qDebug() << "modelAboutToBeReset";
}

void MainWindow::modelReset(){
    currentChanged(QModelIndex(),QModelIndex());
      qDebug() << "modelReset";
}


void MainWindow::currentChanged ( const QModelIndex & current, const QModelIndex & ){

    Engine::TypeItem typeItem=modelB->getTypeItem(current);

    if (typeItem==Engine::TypeItem::PROCEEDABLE){
        ui->widget_3->setProceedable(modelB,current);
     } else if (typeItem==Engine::TypeItem::DIR){
        ui->widget_3->setDir();
    } else if (typeItem==Engine::TypeItem::PROCEEDED){
        ui->widget_3->setProceeded(modelB,current);
     } else {
        ui->widget_3->reset();
    }
}



#include "settingdialog.h"







MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_toolButtonExit_clicked()
{
    QApplication::exit();
}



void MainWindow::on_toolButtonChooseFolder_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()){
        FileEngine* fileEngine=qobject_cast<FileEngine*>(modelB);
        if (fileEngine != nullptr){
            fileEngine->cd(dialog.selectedFiles().at(0));
        }
    }
}


void MainWindow::on_toolButtonSettings_clicked()
{
  /*  SettingDialog d;
    d.exec();
    */
    ConfigDialog dialog;
   /* return*/ dialog.exec();
}
