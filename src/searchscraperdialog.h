#ifndef SEARCHSCRAPERDIALOG_H
#define SEARCHSCRAPERDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include "scrapers/scraper.h"
#include "scanner/scanner.h"

namespace Ui {
class SearchScraperDialog;
}

class SearchScraperDialog : public QDialog
{
    Q_OBJECT

public:
    SearchScraperDialog(QWidget *parent, const QFileInfo& f, QList<Scraper *> scrapers, QNetworkAccessManager *manager);
    SearchScraperDialog(QWidget *parent, const FoundResult& foundResult, QList<Scraper *> scrapers, QNetworkAccessManager *manager);
    ~SearchScraperDialog();

    FoundResult getResult() const;

private:
    Ui::SearchScraperDialog *ui;
    QNetworkAccessManager* m_manager;
    void accept(Scraper *scraper, FilmPtr filmPtr) ;
    void accept(Scraper *scraper, ShowPtr showPtr);
    void init(QList<Scraper*> scrapers);

    FoundResult result;
    QPushButton* findButton=nullptr;

private slots:
    void searchScraper();
    void found(FilmPrtList result);
    void found(ShowPtrList shows);
    void updateMenu();

};

#endif // SEARCHSCRAPERDIALOG_H
