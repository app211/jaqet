#include "scraper.h"

#include <QStringList>

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QApplication>

#include "../inprogressdialog.h"

Scraper::Scraper(QObject *parent)
    :QObject(parent)
{  
    connect(this, SIGNAL(found(FilmPrtList)), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(found(ShowPtrList)), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(scraperError()), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(scraperError(const QString&)), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(found(const Scraper* ,SearchMovieInfo )), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(found(const Scraper* ,SearchEpisodeInfo )), this,
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


void  Scraper::findMovieInfo(QNetworkAccessManager *manager, const QString& movieCode) const {

    p=InProgressDialog::create();

    internalFindMovieInfo(manager,  movieCode);
}

void  Scraper::findEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode) const {

    p=InProgressDialog::create();

    internalFindEpisodeInfo(manager, showCode, season,  episode);
}


void Scraper::closeDialog(){
    if (p!=nullptr){
        p->closeAndDeleteLater();
        p = nullptr;
    }
}

