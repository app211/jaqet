#include "searchscraperdialog.h"
#include "ui_searchscraperdialog.h"
#include <QMenu>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>

#include "fileparser.h"
#include "scanner/filenamescanner.h"
#include "chooseitemdialog.h"


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

    FileNameScanner fns;
    analyse=fns.analyze(fileInfo);

    QMenu *menuFichier = new QMenu(this);

    ui->labelFilename->setText(fileInfo.fileName());
    if (analyse.mediaType.isValidForTV()){
        ui->radioTV->setChecked(true);
        ui->spinBoxSeason->setValue(analyse.mediaType.season);
        ui->spinBoxEpisode->setValue(analyse.mediaType.episode);
    } else {
        ui->radioMovie->setChecked(true);

    }
    foreach (Scraper* scraper,scrapers){
        QAction* scraperAction = new QAction(scraper->getIcon(),scraper->getName(), this);
        scraperAction->setData(qVariantFromValue((void*)scraper));
        menuFichier->addAction(scraperAction);

        connect(scraperAction, SIGNAL(triggered()), this,
                SLOT(searchScraper()));

        connect(scraper, SIGNAL(found(FilmPrtList)), this,
                SLOT(found(FilmPrtList)));

        connect(scraper, SIGNAL(found(ShowPtrList )), this,
                SLOT(found(ShowPtrList )));

    }


    QPushButton* findButton = new QPushButton(tr("&Search"));

    findButton->setMenu(menuFichier);

    ui->buttonBox->addButton(findButton, QDialogButtonBox::AcceptRole);

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

    if (analyse.mediaType.isValidForTV()){
        scraper->searchTV(m_manager,ui->labelAllo->text());
    } else {
        scraper->searchFilm(m_manager,ui->labelAllo->text());
    }
}


void SearchScraperDialog::found(FilmPrtList result){
    Scraper *scraper = qobject_cast<Scraper *>(this->sender());

    if (result.size()>1){
        ChooseItemDialog ch(this);
        ch.setList(result);
        if (ch.exec()==QDialog::Accepted){
            accept(scraper, ch.getSelectedFilm());
        }
    } else if (result.size()==1){
        qDebug() << result.at(0)->title;
        accept(scraper, result.at(0));
    } else {
        QMessageBox::information(this, tr("My Application"),
                                        tr("Nothing found"));
    }
}

void SearchScraperDialog::accept(Scraper *scraper, FilmPtr filmPtr) {
    if (!filmPtr.isNull()){
        result= FoundResult(scraper, filmPtr->code);
        done(QDialog::Accepted);
    }
}

void SearchScraperDialog::accept(Scraper *scraper, ShowPtr showPtr) {
    if (!showPtr.isNull()){
        result= FoundResult(scraper, showPtr->code, analyse.mediaType.season, analyse.mediaType.episode);
        done(QDialog::Accepted);
    }
}

void SearchScraperDialog::found(ShowPtrList shows){
    Scraper *scraper = qobject_cast<Scraper *>(this->sender());

    if (shows.size()>1){
        ChooseItemDialog ch;
        ch.setList(shows);
        if (ch.exec()==QDialog::Accepted){
            accept(scraper, ch.getSelectedShow());
        }
    } else if (shows.size()==1){
        accept(scraper, shows.at(0));
    }
}

SearchScraperDialog::FoundResult SearchScraperDialog::getResult() const {
    return result;
}

SearchScraperDialog::~SearchScraperDialog()
{
    delete ui;
}
