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
            SLOT(closeDialog()));
    connect(this, SIGNAL(found(ShowPtrList)), this,
            SLOT(closeDialog()));
    connect(this, SIGNAL(scraperError()), this,
            SLOT(closeDialog()));
}


InProgressDialog* p=nullptr;

void Scraper::searchFilm( QNetworkAccessManager* manager, const QString& toSearch) const {

    p=InProgressDialog::create();

    internalSearchFilm( manager, toSearch,"fr");
}

void Scraper::searchTV( QNetworkAccessManager* manager, const QString& toSearch)  {

    p=InProgressDialog::create();

    internalSearchTV( manager, toSearch,"fr");
}

void Scraper::closeDialog(){
    if (p!=nullptr){
        p->closeAndDeleteLater();
        p = nullptr;
    }
}

