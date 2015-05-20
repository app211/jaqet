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
    SearchScraperDialog(QWidget *parent, const CurrentItemData &foundResult, QList<Scraper *> scrapers, QNetworkAccessManager *manager);
    ~SearchScraperDialog();

    FoundResult getResult() const;

private:
    Ui::SearchScraperDialog *ui;
    QNetworkAccessManager* m_manager;
    void accept(Scraper *scraper, const FilmPtr& filmPtr) ;
    void accept(Scraper *scraper, const ShowPtr& showPtr, const int season, const int episode);
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
