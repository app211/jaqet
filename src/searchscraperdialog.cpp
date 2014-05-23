#include "searchscraperdialog.h"
#include "ui_searchscraperdialog.h"
#include <QMenu>
#include <QDebug>

#include "fileparser.h"

SearchScraperDialog::SearchScraperDialog(QWidget *parent,const QFileInfo& fileInfo, QList<Scraper*> scrapers, QNetworkAccessManager* manager) :
    QDialog(parent),
    ui(new Ui::SearchScraperDialog),
    m_manager(manager)
{
    ui->setupUi(this);

    QString baseName = FileParser::baseName(fileInfo);
    QString name = FileParser::cleanName(baseName);
    QString filteredName = FileParser::filterBlacklist(name);
    ui->labelAllo->setText(filteredName);

    // _tvShowTV = FileParser::isSeries(baseName,_seasonTV,_episodeTV);
    //ui->labelType->setText(_tvShowTV?"T":"M");

    QMenu *menuFichier = new QMenu(this);

    foreach (Scraper* scraper,scrapers){
        QAction* scraperAction = new QAction(scraper->getIcon(),"&TMDB", this);
        scraperAction->setData(qVariantFromValue((void*)scraper));
        menuFichier->addAction(scraperAction);

        connect(scraperAction, SIGNAL(triggered()), this,
                SLOT(searchScraper()));

        connect(scraper, SIGNAL(found(FilmPrtList)), this,
                SLOT(found(FilmPrtList)));


    }

    ui->pushButtonSearchScraper->setMenu(menuFichier);
}

void SearchScraperDialog::searchScraper(){

    QAction *action = qobject_cast<QAction *>(this->sender());

    if(action == nullptr)
    {
        return;
    }

    action->data();

    Scraper* scraper= (Scraper*)action->data().value<void *>();
    if(scraper == nullptr)
    {
        return;
    }

    ui->comboBoxProposition->clear();

    if (true /*!this->_tvShowTV*/){
        SearchResult a;
        scraper->searchFilm(m_manager,ui->labelAllo->text());
        /* {
            for (int i=0; i<(a.films.size());i++){
                ui->comboBoxProposition->addItem(a.films.at(i)->title+" "+ a.films.at(i)->productionYear, a.films.at(i)->code);

            }
        }*/
    } else {
        SearchTVResult a;
        if (scraper->searchTV(ui->labelAllo->text(),a)){
            for (int i=0; i<(a.shows.size());i++){
                ui->comboBoxProposition->addItem(a.shows.at(i).title+" "+ a.shows.at(i).productionYear, a.shows.at(i).code);
                qDebug() << a.shows.at(i).originalTitle << " " << a.shows.at(i).productionYear << " " << a.shows.at(i).posterHref << a.shows.at(i).code;
                /*   */
            }
        }

    }

    //ui->scrollAreaScrapResult->setVisible(false);

}

void SearchScraperDialog::proceed(Scraper *scraper){
    if (ui->comboBoxProposition->currentIndex()>=0){
        QString code = ui->comboBoxProposition->itemData( ui->comboBoxProposition->currentIndex()).toString();
        emit proceed(scraper, code);
        close();
    }
}

void SearchScraperDialog::found(FilmPrtList result){
    for (int i=0; i<(result.size());i++){
        ui->comboBoxProposition->addItem(result.at(i)->title+" "+ result.at(i)->productionYear, result.at(i)->code);

    }

    ui->comboBoxProposition->setEnabled(true);
    ui->pushButtonSearchFilm->setEnabled(true);

    Scraper *scraper = qobject_cast<Scraper *>(this->sender());

    ui->pushButtonSearchFilm->disconnect();
    QObject::connect(ui->pushButtonSearchFilm, &QPushButton::released, [=]()
    {
        proceed(scraper);
    });
}
SearchScraperDialog::~SearchScraperDialog()
{
    delete ui;
}
