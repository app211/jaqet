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

#include "template/templateyadis.h"

#include "av/avprobe.h"
#include "tvixengine.h"

#include <QProgressDialog>



void MainWindow::doubleClicked ( const QModelIndex & index){

    if (index.isValid()){
        QVariant v=modelB->data(index,Qt::DisplayRole);
        if (v==".."){
            modelB->cdUp();
        } else {
            modelB->cd(v.toString());

        }
    }
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    // b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");
    // b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");

   // b.loadTemplate("/home/teddy/Developpement/Maxx Shiny/template.xml");
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
            this, SLOT(doubleClicked ( const QModelIndex & )));

    //   ui->listView->setRootIndex(f->mapFromSource(ffdf->setRootPath(QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0))));

    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));

    connect(&b, SIGNAL(tivxOk(QPixmap )), this, SLOT(s_clicked_texte(QPixmap )));

    //connect(ui->label, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));

    /*Scraper* s=new AlloCineScraper;
    allocineAction = new QAction(s->getIcon(),"&Allocine", this);
    allocineAction->setData(qVariantFromValue((void*)s));
*/
    Scraper* s=new TheMovieDBScraper;
    tmdbAction = new QAction(s->getIcon(),"&TMDB", this);
    tmdbAction->setData(qVariantFromValue((void*)s));

    this->scrapes.append(s);
    /*  s=new TheTVDBScraper;
    tvdbAction = new QAction(s->getIcon(),"TvDb", this);
    tvdbAction->setData(qVariantFromValue((void*)s));
*/
    /*  connect(allocineAction, SIGNAL(triggered()), this,
            SLOT(searchScraper()));*/
/*    connect(tmdbAction, SIGNAL(triggered()), this,
            SLOT(searchScraper()));*/
    /*    connect(tvdbAction, SIGNAL(triggered()), this,
            SLOT(searchScraper()));
*/
    connect(s, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
           SLOT(test(const Scraper*,SearchMovieInfo)));

    connect(s, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
           SLOT(test2(const Scraper*,SearchEpisodeInfo)));

    QMenu *menuFichier = new QMenu(this);
    //menuFichier->addAction(allocineAction);
    menuFichier->addAction(tmdbAction);
    //  menuFichier->addAction(tvdbAction);

    scene = new QGraphicsScene(this);
    ui->graphicsViewPosters->setScene(scene);

    //ui->scrollAreaScrapResult->setVisible(false);
}

void MainWindow::ctxMenu(const QPoint &pos) {
    QMenu *menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    //menu->exec(ui->label->mapToGlobal(pos));
}

#include "searchscraperdialog.h"

void MainWindow::search(QFileInfo f){
    qDebug() << f.absoluteFilePath();

    SearchScraperDialog fd(this, f , this->scrapes, &this->manager);
    if (fd.exec()==QDialog::Accepted){
        if (!fd.getResult().isNull()){
            if (!fd.getResult().isTV()){
                fd.getResult().getScraper()->findMovieInfo(&this->manager,fd.getResult().getCode());
            } else {
                fd.getResult().getScraper()->findEpisodeInfo(&this->manager,fd.getResult().getCode(),fd.getResult().getSeason(),fd.getResult().getEpisode());
            }
        }
    }

}


void MainWindow::currentChanged ( const QModelIndex & current, const QModelIndex & previous ){


    Engine::TypeItem typeItem=modelB->getTypeItem(current);

    if (typeItem==Engine::TypeItem::PROCEEDABLE){
        ui->stackedWidget->setCurrentIndex(0);

        fileInfo=modelB->fileInfo(current);

        ui->Proceed->disconnect();
        QObject::connect(ui->Proceed, &QPushButton::released, [=]()
        {
            search(fileInfo);
        });

    } else if (typeItem==Engine::TypeItem::DIR){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

QPixmap createDefaultPoster(int w, int h){
    QPixmap result(w,h);
    result.fill(Qt::white);

    QPainter pixPaint(&result);

    QPixmap icon;
    icon.load(":/DownloadIcon.png");
    pixPaint.drawPixmap((w-icon.width())/2,(h-icon.height())/2,icon.width(),icon.height(),icon);

    return result;
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

#include "promise.h"


void MainWindow::test2(const Scraper* scraper,SearchEpisodeInfo b){
    qDebug() << b.code << b.title;
    ui->labelMovieScraper->setPixmap(scraper->getIcon().pixmap(16));
    ui->stackedWidget->setCurrentIndex(1);
    ui->synopsis->setText(b.synopsis);

    ui->labelEpisodeTitle->setVisible(true);
    ui->labelSeasonEpisode->setVisible(true);

    ui->labelEpisodeTitle->setText(b.title);

    ui->labelSeasonEpisode->setText(QString("Season %1 - Episode %2").arg(b.season).arg(b.episode));
}

void MainWindow::test(const Scraper* scraper,SearchMovieInfo b){
    ui->labelEpisodeTitle->setVisible(false);
    ui->labelSeasonEpisode->setVisible(false);

    ui->labelMovieScraper->setPixmap(scraper->getIcon().pixmap(16));
    if (!b.linkHref.isEmpty()){
        ui->labelMovieTitle->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
        ui->labelMovieTitle->setTextFormat(Qt::RichText);
        ui->labelMovieTitle->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelMovieTitle->setOpenExternalLinks(true);
    } else {
        ui->labelMovieTitle->setText(b.title);
    }

    ui->stackedWidget->setCurrentIndex(1);
    if (!b.linkHref.isEmpty()){
        ui->labelUrl->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
        ui->labelUrl->setTextFormat(Qt::RichText);
        ui->labelUrl->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->labelUrl->setOpenExternalLinks(true);
    }

    ui->synopsis->setText(b.synopsis);

    ui->toolButtonSysnopsis->disconnect();
    QObject::connect(ui->toolButtonSysnopsis, &QPushButton::released, [=]()
    {
        setMovieInfo(b);
    });

    scene->clear();


    ui->graphicsViewPosters->setScene(NULL);

    int x=0;
    int y=20;
    int w=200;
    int h=200;

    QSet<QString> urls;

    if (!b.postersHref.isEmpty()){
        foreach (const QString& url , b.postersHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(w,h));
            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            Promise* promise=Promise::loadAsync(manager,realUrl);
            QObject::connect(promise, &Promise::completed, [=]()
            {
                if (promise->reply->error() ==QNetworkReply::NoError){
                    QByteArray qb=promise->reply->readAll();
                    setImageFromInternet( qb, pi,  x,  y,  w,  h);
                }
            });

            QPushButton* b = new QPushButton("Plus");

            QObject::connect(b, &QPushButton::released, [=]()
            {
                setPoster(url,scraper);
            });

            QGraphicsProxyWidget* button = scene->addWidget(b);
            button->setPos(x,h);

            x+=w+10;

        }
    }

    if (!b.backdropsHref.isEmpty()){
        foreach (const QString& url , b.backdropsHref){

            QString realUrl=scraper->getBestImageUrl(url,QSize(w,h));
            if (urls.contains(realUrl)){
                continue;
            }

            urls.insert(realUrl);

            scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

            QPixmap scaled = createDefaultPoster(w,h);

            QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
            pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

            Promise* promise=Promise::loadAsync(manager,realUrl);
            QObject::connect(promise, &Promise::completed, [=]()
            {
                if (promise->reply->error() ==QNetworkReply::NoError){
                    QByteArray qb=promise->reply->readAll();
                    setImageFromInternet( qb, pi,  x,  y,  w,  h);
                }
            });

            QPushButton* b = new QPushButton("Plus");

            QObject::connect(b, &QPushButton::released, [=]()
            {
                setBackdrop(url,scraper);
            });

            QGraphicsProxyWidget* button = scene->addWidget(b);

            button->setPos(x,h);

            x+=w+10;

        }
    }

    ui->graphicsViewPosters->setScene(scene);

    //ui->scrollAreaScrapResult->setVisible(true);


}

void MainWindow::buildTvix() {
    b.createTivx(this->manager,_poster, _backdrop,_texts);
    //   b.canceled();
}

void MainWindow::s_clicked_texte(QPixmap result){
    ui->labelPoster->setPixmap(result);
}

void MainWindow::setPoster (const QString& url, const Scraper *_currentScrape){


    this->_poster=ScraperResource(url,_currentScrape);

    buildTvix();

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

void MainWindow::setBackdrop(const QString& url, const Scraper *_currentScrape){

    this->_backdrop=ScraperResource(url,_currentScrape);

    buildTvix();

}

void MainWindow::chooseTizzBirdFolder() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()){
        modelB->cd(dialog.selectedFiles().at(0));
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

    ui->labelPoster->pixmap()->save(QFileInfo(QDir(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0)),"test.png").absoluteFilePath());
}
