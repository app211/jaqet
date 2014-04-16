#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include <curl/curl.h>
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

#include "scrapers/allocinescraper.h"
#include "scrapers/themoviedbscraper.h"
#include "filedownloader.h"
#include "myproxymodel.h"

#include "webfile.h"
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
templateYadis b;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    b.loadTemplate("/home/teddy/Developpement/Tribute Glass Mix/template.xml");
    //b.loadTemplate("/home/teddy/Developpement/POLAR/template.xml");
    //  b.loadTemplate("/home/teddy/Developpement/CinemaView/template.xml");
    //   b.loadTemplate("/home/teddy/Developpement/Relax 2/template.xml");
    b.loadTemplate("C:/Program Files (x86)/yaDIS/templates/Origins/template.xml");
    // Create seed for the random
    // That is needed only once on application startup
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    ui->setupUi(this);


    modelB=new MyQStringListModel(this, QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0));

    QSortFilterProxyModel* f =new QSortFilterProxyModel(this);
    f->setSourceModel(modelB);

    ui->listView->setModel(f);
    connect(ui->listView, SIGNAL(doubleClicked ( const QModelIndex & )),
            this, SLOT(doubleClicked ( const QModelIndex & )));

    //   ui->listView->setRootIndex(f->mapFromSource(ffdf->setRootPath(QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation).at(0))));

    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));

    //connect(ui->label, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));

    allocineAction = new QAction("&Allocine", this);
    allocineAction->setData(qVariantFromValue((void*)new AlloCineScraper));
    tmdbAction = new QAction("&TMDB", this);
    tmdbAction->setData(qVariantFromValue((void*)new TheMovieDBScraper));

    connect(allocineAction, SIGNAL(triggered()), this,
            SLOT(searchScraper()));
    connect(tmdbAction, SIGNAL(triggered()), this,
            SLOT(searchScraper()));

    QMenu *menuFichier = new QMenu(this);
    menuFichier->addAction(allocineAction);
    menuFichier->addAction(tmdbAction);

    ui->pushButtonSearchScraper->setMenu(menuFichier);

    scene = new QGraphicsScene(this);
    ui->graphicsViewPosters->setScene(scene);

    //ui->scrollAreaScrapResult->setVisible(false);
}

void MainWindow::ctxMenu(const QPoint &pos) {
    QMenu *menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    //menu->exec(ui->label->mapToGlobal(pos));
}




void MainWindow::currentChanged ( const QModelIndex & current, const QModelIndex & previous ){

    ui->comboBoxProposition->clear();
    ui->comboBoxProposition->setEnabled(false);
    ui->pushButtonSearchFilm->setEnabled(false);

    if (!modelB->isDir(current)){

        //ui->scrollAreaScrapResult->setVisible(false);

        fileInfo=modelB->fileInfo(current);

        QString baseName = FileParser::baseName(fileInfo);
        QString name = FileParser::cleanName(baseName);
        QString filteredName = FileParser::filterBlacklist(name);
        ui->labelAllo->setText(filteredName);

        _tvShowTV = FileParser::isSeries(baseName);
        ui->labelType->setText(_tvShowTV?"T":"M");

        _episodeTV="01";
        _seasonTV="01";

                // static avprobe av;
                //  av.getInfo(fileInfo.absoluteFilePath());



    }
}



void MainWindow::searchScraper(){

    QAction *action = qobject_cast<QAction *>(this->sender());

    if(action == 0)
    {
        return;
    }

    action->data();

    Scraper* scraper= (Scraper*)action->data().value<void *>();
    if(scraper == 0)
    {
        return;
    }

    this->currentScraper = scraper;

    ui->comboBoxProposition->clear();

    if (!this->_tvShowTV){
        SearchResult a;
        if (currentScraper->searchFilm(ui->labelAllo->text(),a)){
            for (int i=0; i<(a.films.size());i++){
                ui->comboBoxProposition->addItem(a.films.at(i).title+" "+ a.films.at(i).productionYear, a.films.at(i).code);
                qDebug() << a.films.at(i).originalTitle << " " << a.films.at(i).productionYear << " " << a.films.at(i).posterHref << a.films.at(i).code;
                /*   */
            }
        }
    } else {
        SearchTVResult a;
        if (currentScraper->searchTV(ui->labelAllo->text(),a)){
            for (int i=0; i<(a.shows.size());i++){
                ui->comboBoxProposition->addItem(a.shows.at(i).title+" "+ a.shows.at(i).productionYear, a.shows.at(i).code);
                qDebug() << a.shows.at(i).originalTitle << " " << a.shows.at(i).productionYear << " " << a.shows.at(i).posterHref << a.shows.at(i).code;
                /*   */
            }
        }

    }

    //ui->scrollAreaScrapResult->setVisible(false);

    ui->comboBoxProposition->setEnabled(true);
    ui->pushButtonSearchFilm->setEnabled(true);
}

bool loadPixmap(const QString& url, QPixmap& pixmap){
    webfile f;
    f.setUrl(url);

    qDebug()<< url;

    if (f.open())
    {
        QByteArray b;
        char buffer[1024];
        qint64  nSize = 0;


        while ((nSize = f.read(buffer, sizeof(buffer))) > 0)
        {
            b.append(buffer,nSize);
        }

        f.close();

        if (f.error())
        {
            return false; // m_webfile.errorString()
        }

        pixmap.loadFromData(b);
    } else {
        return false; // m_webfile.errorString()
    }

    return true;
}

void MainWindow::changeFileName(){
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("User name:"), QLineEdit::Normal,
                                         ui->labelAllo->text(), &ok);
    if (ok && !text.isEmpty()){
        ui->labelAllo->setText(text);
    }
}

void MainWindow::searchAllocineMovie(){
    QString code = ui->comboBoxProposition->itemData( ui->comboBoxProposition->currentIndex()).toString();


    QProgressDialog progress("Task in progress...", "Cancel", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();


    if (this->_tvShowTV){
        SearchEpisodeInfo b;
        if (!currentScraper->findEpisodeInfo(code,this->_seasonTV,this->_episodeTV,b)){

        }
    } else {
        SearchMovieInfo b;
        if (!currentScraper->findMovieInfo(code,b)){

        } else{

            progress.setMaximum(b.postersHref.size()+b.backdropsHref.size());

            int curVal=0;

            progress.setValue(curVal++);

            if (!b.linkHref.isEmpty()){
                ui->labelUrl->setText(QString("<a href=\"").append(b.linkHref).append("\">").append(b.linkName).append("</a>"));
                ui->labelUrl->setTextFormat(Qt::RichText);
                ui->labelUrl->setTextInteractionFlags(Qt::TextBrowserInteraction);
                ui->labelUrl->setOpenExternalLinks(true);
            }

            ui->synopsis->setText(b.synopsis);
            /*  if (!b.posterHref.isEmpty()){
        QPixmap buttonImage;
        loadPixmap(b.posterHref,buttonImage);
        ui->label->setPixmap(buttonImage);
    }
*/
            scene->clear();



            ui->graphicsViewPosters->setScene(NULL);

            int x=0;
            int y=20;
            int w=200;
            int h=200;

            if (!b.postersHref.isEmpty()){
                foreach (const QString& url , b.postersHref){
                    progress.setValue(curVal++);
                    QPixmap buttonImage;
                    loadPixmap(currentScraper->getBestImageUrl(url,QSize(w,h)),buttonImage);

                    scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

                    QPixmap scaled = (buttonImage.width()>w || buttonImage.height()>h) ? buttonImage.scaled(w,h,Qt::KeepAspectRatio):buttonImage;

                    QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
                    pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

                    QPushButton* b = new QPushButton("Plus");

                    QObject::connect(b, &QPushButton::released, [=]()
                    {
                        setPoster(url,currentScraper);
                    });
                    QGraphicsProxyWidget* button = scene->addWidget(b);
                    button->setPos(x,h);

                    x+=w+10;

                    if (progress.wasCanceled())
                        return;

                }
            }

            if (!b.backdropsHref.isEmpty()){
                foreach (const QString& url , b.backdropsHref){
                    progress.setValue(curVal++);

                    QPixmap buttonImage;
                    loadPixmap(currentScraper->getBestImageUrl(url,QSize(w,h)),buttonImage);

                    scene->addRect(x,y,w,h, QPen(QBrush(Qt::BDiagPattern),1),QBrush(Qt::BDiagPattern));

                    QPixmap scaled = (buttonImage.width()>w || buttonImage.height()>h) ? buttonImage.scaled(w,h,Qt::KeepAspectRatio):buttonImage;

                    QGraphicsPixmapItem* pi=scene->addPixmap(scaled);
                    pi->setPos(x+(w-scaled.width())/2,y+(h-scaled.height())/2);

                    QPushButton* b = new QPushButton("Plus");

                    QObject::connect(b, &QPushButton::released, [=]()
                    {
                        setBackdrop(url,currentScraper);
                    });

                    QGraphicsProxyWidget* button = scene->addWidget(b);

                    button->setPos(x,h);

                    x+=w+10;

                    if (progress.wasCanceled())
                        return;
                }
            }

            ui->graphicsViewPosters->setScene(scene);

            //ui->scrollAreaScrapResult->setVisible(true);
        }
    }
}

void MainWindow::buildTvix() {
    ui->labelPoster->setPixmap(b.createTivx(_poster, _backdrop,_synopsis));
}

void MainWindow::setPoster (const QString& url, Scraper *_currentScrape){


    this->_poster=ScraperResource(url,_currentScrape);

    buildTvix();

}

void MainWindow::setSynopsis(){

    this->_synopsis=this->ui->synopsis->toPlainText();

    buildTvix();

}

void MainWindow::setBackdrop(const QString& url, Scraper *_currentScrape){

    this->_backdrop=ScraperResource(url,_currentScrape);

    buildTvix();

}
void MainWindow::chooseTizzBirdFolder() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()){
        modelB->setPath(dialog.selectedFiles().at(0));
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
