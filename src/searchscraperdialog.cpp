#include "searchscraperdialog.h"
#include "ui_searchscraperdialog.h"
#include <QMenu>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>

#include "scanner/filenamescanner.h"
#include "chooseitemdialog.h"


SearchScraperDialog::SearchScraperDialog(QWidget *parent,const QFileInfo& fileInfo, QList<Scraper*> scrapers, QNetworkAccessManager* manager) :
    QDialog(parent),
    ui(new Ui::SearchScraperDialog),
    m_manager(manager)

{
    ui->setupUi(this);

    init(scrapers);

    FileNameScanner fns;
    Scanner::AnalysisResult analyse=fns.analyze(fileInfo);

    ui->lineEditTitle->setText(fns.getFilteredName(fileInfo));

    if (analyse.mediaType.year>=1900){
        ui->dateEdit->setDate(QDate(analyse.mediaType.year,1,1));
        ui->checkBoxUseYear->setChecked(true);
    } else {
        ui->dateEdit->setDate(QDate());
        ui->checkBoxUseYear->setChecked(false);
    }

    ui->labelFilename->setText(fileInfo.fileName());
    if (analyse.mediaType.isValidForTV()){
        ui->radioTV->setChecked(true);
        ui->spinBoxSeason->setValue(analyse.mediaType.season);
        ui->spinBoxEpisode->setValue(analyse.mediaType.episode);
    } else {
        ui->radioMovie->setChecked(true);

    }
}

static QString language()
{
#ifdef QT_DEBUG
    return "fr";
#else
    // QLocale::name returns the locale in lang_COUNTRY format
    // we only need the 2 letter lang code

    return  QLocale::system().name().left(2);
#endif
}

void SearchScraperDialog::updateMenu(){
    if (this->findButton != nullptr){
        QMenu *menuFichier = this->findButton->menu();
        if (menuFichier != nullptr){
            for( QAction* action: menuFichier->actions()){
                Scraper* scraper= (Scraper*)action->data().value<void *>();
                if(scraper != nullptr)
                {
                    if (ui->radioMovie->isChecked()){
                        action->setEnabled(scraper->haveCapability(Scraper::Movie) && scraper->supportLanguage(language()));
                    } else {
                        action->setEnabled(scraper->haveCapability(Scraper::TV) && scraper->supportLanguage(language()));
                    }
                }
            }
        }
    }
}

SearchScraperDialog::SearchScraperDialog(QWidget *parent, const CurrentItemData& foundResult, QList<Scraper*> scrapers, QNetworkAccessManager* manager) :
    QDialog(parent),
    ui(new Ui::SearchScraperDialog),
    m_manager(manager)

{
    ui->setupUi(this);

    init(scrapers);

    ui->lineEditTitle->setText(foundResult.originalTitle());

    if (foundResult.isTV()){
        ui->radioTV->setChecked(true);
        ui->spinBoxSeason->setValue(foundResult.season());
        ui->spinBoxEpisode->setValue(foundResult.episode());
    } else {
        ui->radioMovie->setChecked(true);
    }

    if (!foundResult.year()>1900){
        ui->dateEdit->setDate(QDate(foundResult.year(),1,1));
        ui->checkBoxUseYear->setChecked(true);
     } else {
        ui->dateEdit->setDate(QDate());
        ui->checkBoxUseYear->setChecked(false);
       }

}

void SearchScraperDialog::init(QList<Scraper*> scrapers){
    QMenu *menuFichier = new QMenu(this);

    foreach (Scraper* scraper,scrapers){
        QAction* scraperAction = new QAction(scraper->icon(),scraper->name(), this);
        scraperAction->setData(qVariantFromValue((void*)scraper));
        menuFichier->addAction(scraperAction);

        connect(scraperAction, SIGNAL(triggered()), this,
                SLOT(searchScraper()));

        connect(scraper, SIGNAL(found(FilmPrtList)), this,
                SLOT(found(FilmPrtList)));

        connect(scraper, SIGNAL(found(ShowPtrList )), this,
                SLOT(found(ShowPtrList )));

    }


    findButton = new QPushButton(tr("&Search"));

    findButton->setMenu(menuFichier);

    ui->buttonBox->addButton(findButton, QDialogButtonBox::AcceptRole);
}

void SearchScraperDialog::searchScraper(){

    QAction *action = qobject_cast<QAction *>(this->sender());

    if(action == nullptr)
    {
        return;
    }

    Scraper* scraper= (Scraper*)action->data().value<void *>();
    if(scraper == nullptr)
    {
        return;
    }

    int year=ui->checkBoxUseYear->isChecked()?ui->dateEdit->date().year():-1;

    if (ui->radioTV->isChecked()){
        scraper->searchTV(m_manager,ui->lineEditTitle->text(),language());
    } else {
        scraper->searchFilm(m_manager,ui->lineEditTitle->text(), year,language());
    }
}

void SearchScraperDialog::found(FilmPrtList result){
    Scraper *scraper = qobject_cast<Scraper *>(this->sender());

    if (result.size()>1){
        FilmPrtList candidates;

        // Filter result by year
        if (ui->checkBoxUseYear->isChecked()){
            QDate d=ui->dateEdit->date();
            if( d.isValid() && d.year()>=1900){
            for (FilmPtr film : result){
                bool bOk;
                int productionYear= film->productionYear.toInt(&bOk);
                if (bOk && productionYear==d.year()){
                    candidates.append(film);
                }
            }
            }
        }

        if (candidates.size()==1){
            accept(scraper, candidates.at(0));
        } else {
            ChooseItemDialog ch(this);
            ch.setList(result);
            if (ch.exec()==QDialog::Accepted){
                accept(scraper, ch.getSelectedFilm());
            }
        }
    } else if (result.size()==1){
        accept(scraper, result.at(0));
    } else {
        QMessageBox::information(this, tr("My Application"),
                                 tr("Nothing found"));
    }
}

void SearchScraperDialog::accept(Scraper *scraper, const FilmPtr& filmPtr) {
    if (!filmPtr.isNull()){
        result= FoundResult(scraper, filmPtr,language());
        done(QDialog::Accepted);
    }
}

void SearchScraperDialog::accept(Scraper *scraper, const ShowPtr& showPtr, const int season, const int episode) {
    if (!showPtr.isNull()){
        result= FoundResult(scraper, showPtr,season,episode,language());
        done(QDialog::Accepted);
    }
}

void SearchScraperDialog::found(ShowPtrList shows){
    Scraper *scraper = qobject_cast<Scraper *>(this->sender());

    if (shows.size()>1){
        ChooseItemDialog ch;
        ch.setList(shows);
        if (ch.exec()==QDialog::Accepted){
            accept(scraper, ch.getSelectedShow(),ui->spinBoxSeason->value(),ui->spinBoxEpisode->value());
        }
    } else if (shows.size()==1){
        accept(scraper, shows.at(0), ui->spinBoxSeason->value(),ui->spinBoxEpisode->value());
    } else {
        QMessageBox::information(this, tr("My Application"),
                                 tr("Nothing found"));
    }
}

FoundResult SearchScraperDialog::getResult() const {
    return result;
}

SearchScraperDialog::~SearchScraperDialog()
{
    delete ui;
}
