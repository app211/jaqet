#include "scraper.h"

#include <QStringList>

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QApplication>

#include "../inprogressdialog.h"

Scraper::Scraper()
{  
    connect(this, SIGNAL(found(FilmPrtList)), this,
            SLOT(error()));
    connect(this, SIGNAL(scraperError()), this,
            SLOT(error()));
}


InProgressDialog* p=nullptr;

void Scraper::searchFilm( QNetworkAccessManager* manager, const QString& toSearch) const {

    p=InProgressDialog::create();

    internalSearchFilm( manager, toSearch);
}

void Scraper::error(){
    if (p!=nullptr){
        p->closeAndDeleteLater();
        p = nullptr;
    }
}

