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

#include "scrapers/allocinescraper.h"
#include "scrapers/themoviedbscraper.h"
#include "scrapers/thetvdbscraper.h"

#include "filedownloader.h"

#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QCoreApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include "fileparser.h"


#include "engine/tvixengine.h"
#include "promise.h"
#include "searchscraperdialog.h"

#include <QProgressDialog>



void MainWindow::doubleClicked ( const QModelIndex & index){

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
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
#ifdef Q_OS_WIN32
 //    b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");
#else
  //  b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    // b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");

   // b.loadTemplate("/home/teddy/Developpement/Maxx Shiny/template.xml");
#endif
    // Create seed for the random
    // That is needed only once on application startup
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    ui->setupUi(this);


    modelB=new TVIXEngine(this, QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0));

    QSortFilterProxyModel* f =new QSortFilterProxyModel(this);
    f->setSourceModel(modelB);

    ui->listView->setModel(f);
    connect(ui->listView, SIGNAL(doubleClicked ( const QModelIndex & )),
            modelB, SLOT(doubleClicked ( const QModelIndex & )));

    //   ui->listView->setRootIndex(f->mapFromSource(ffdf->setRootPath(QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0))));

    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));


    scene = new QGraphicsScene(this);
   // ui->widget_3->setScene(scene);

    //ui->scrollAreaScrapResult->setVisible(false);
}

void MainWindow::ctxMenu(const QPoint &pos) {
    QMenu *menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    //menu->exec(ui->label->mapToGlobal(pos));
}





void MainWindow::currentChanged ( const QModelIndex & current, const QModelIndex & ){

    Engine::TypeItem typeItem=modelB->getTypeItem(current);

    if (typeItem==Engine::TypeItem::PROCEEDABLE){
        ui->widget_3->setProceedable(modelB,current);
     } else if (typeItem==Engine::TypeItem::DIR){
        ui->widget_3->setDir();
    } else if (typeItem==Engine::TypeItem::PROCEEDED){
      //  ui->widget_3->setProceeded(modelB->preview(current)));
     }
}

void MainWindow::setImageFromInternet( QByteArray& qb, QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h){

    if (itemToUpdate==nullptr){
        // Nothing to do
        return;
    } else {
        bool itemSeemsToExistsAnymore=false;
        QList<QGraphicsItem*> all = this->scene->items();
        for (int i = 0; i < all.size(); i++)
        {
            if (itemToUpdate==all[i]){
                itemSeemsToExistsAnymore=true;
                break;
            }
        }

        if (!itemSeemsToExistsAnymore){
            // Nothing to do
            return;
        }
    }

    QPixmap pixmap;
    pixmap.loadFromData(qb);
    QPixmap scaled = (pixmap.width()>w || pixmap.height()>h) ? pixmap.scaled(w,h,Qt::KeepAspectRatio):pixmap;
    itemToUpdate->setPixmap(scaled);
    itemToUpdate->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);
}





void MainWindow::buildTvix() {
    //b.createTivx(this->manager,_poster, _backdrop,_texts);
    //   b.canceled();
}

void MainWindow::s_clicked_texte(QPixmap result){
  //  ui->labelPoster->setPixmap(result);
}


void MainWindow::setMovieInfo( const SearchMovieInfo& searchMovieInfo){

    this->_texts["synopsis"]=searchMovieInfo.synopsis;
    this->_texts["actors"]=searchMovieInfo.actors.join(',');
    this->_texts["directors"]=searchMovieInfo.directors.join(',');

    if (searchMovieInfo.productionYear>0){
        this->_texts["year"]=QString::number(searchMovieInfo.productionYear);
    }

    if (searchMovieInfo.runtime>0){
        this->_texts["runtime"]=QString::number(searchMovieInfo.runtime);
    }

    buildTvix();

}



void MainWindow::chooseTizzBirdFolder() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()){
        //modelB->cd(dialog.selectedFiles().at(0));
    }
}

void MainWindow::loadImage()
{
    QPixmap buttonImage;
    buttonImage.loadFromData(m_pImgCtrl->downloadedData());

    QGraphicsPixmapItem* pi=scene->addPixmap(buttonImage.scaled(100,100,Qt::KeepAspectRatio));
    pi->setOffset(QPoint(10,20));
    scene->addWidget(new QPushButton("Plus"));
    //ui->label->setPixmap(buttonImage);
    m_pImgCtrl->deleteLater();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::savePix(){

 //   ui->labelPoster->pixmap()->save(QFileInfo(QDir(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0)),"test.png").absoluteFilePath());
}
