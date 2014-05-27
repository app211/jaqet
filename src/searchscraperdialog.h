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
    explicit SearchScraperDialog(QWidget *parent, const QFileInfo& f, QList<Scraper *> scrapers, QNetworkAccessManager *manager);
    ~SearchScraperDialog();

    struct FoundResult {

        FoundResult() : isnull(true){
        }

        FoundResult(Scraper *scraper, const QString& code)
            :  isnull(false),
              tv(false),
              code(code),
              scraper(scraper)
              {
        }

        FoundResult(Scraper *scraper, const QString& code, int season, int episode)
            : isnull(false),
              tv(true),
              code(code),
              scraper(scraper),
              season(season),
              episode(episode) {
        }

        bool isNull() const {
            return isnull;
        }

        bool isTV() const {
            return tv;
        }

        QString getCode() const {
            return code;
        }

        Scraper *getScraper() const {
            return scraper;
        }

        int getSeason(){
            return season;
        }

        int getEpisode(){
            return episode;
        }

        bool tv;
        QString code;
        Scraper *scraper;
        /*const*/ bool isnull;
        int season;
        int episode;
    };

    FoundResult getResult() const;

private:
    Ui::SearchScraperDialog *ui;
    QNetworkAccessManager* m_manager;
    Scanner::AnalysisResult analyse;
    void accept(Scraper *scraper, FilmPtr filmPtr) ;
    void accept(Scraper *scraper, ShowPtr showPtr);

    FoundResult result;

private slots:
    void searchScraper();
    void found(FilmPrtList result);
    void found(ShowPtrList shows);
};

#endif // SEARCHSCRAPERDIALOG_H
