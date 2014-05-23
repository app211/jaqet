#ifndef SEARCHSCRAPERDIALOG_H
#define SEARCHSCRAPERDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include "scrapers/scraper.h"

namespace Ui {
class SearchScraperDialog;
}

class SearchScraperDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchScraperDialog(QWidget *parent, const QFileInfo& f, QList<Scraper *> scrapers, QNetworkAccessManager *manager);
    ~SearchScraperDialog();

private:
    Ui::SearchScraperDialog *ui;
    QNetworkAccessManager* m_manager;

private slots:
    void searchScraper();
    void found(FilmPrtList result);
    void proceed(Scraper *scraper);

signals:
    void proceed(Scraper *scraper,const QString& code);

};

#endif // SEARCHSCRAPERDIALOG_H
