#include "scraper.h"

#include <QStringList>

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QApplication>
#include <QMessageBox>
#include <QPointer>

#include "../inprogressdialog.h"

Scraper::Scraper(QObject *parent)
    :QObject(parent)
{  
    connect(this, SIGNAL(found(FilmPrtList)), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(found(ShowPtrList)), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(scraperError()), this,
            SLOT(showErrorDialog()));

    connect(this, SIGNAL(scraperError(const QString&)), this,
            SLOT(showErrorDialog(const QString&)));

    connect(this, SIGNAL(found(const Scraper* ,MediaMovieSearchPtr )), this,
            SLOT(closeDialog()));

    connect(this, SIGNAL(found(const Scraper* ,MediaTVSearchPtr )), this,
            SLOT(closeDialog()));
}


QPointer<InProgressDialog> p;

void Scraper::searchFilm( QNetworkAccessManager* manager, const QString& toSearch, int year) const {

    if (p.isNull()) {
        p=InProgressDialog::create();
    }

    internalSearchFilm( manager, toSearch,"fr", year);
}

void Scraper::searchTV( QNetworkAccessManager* manager, const QString& toSearch)  {

    if (p.isNull()) {
        p=InProgressDialog::create();
    }

    internalSearchTV( manager, toSearch,"fr");
}

void Scraper::showErrorDialog(const QString& error){
    closeDialog();

    QMessageBox msgBox;
    msgBox.setText("Unable to retrieve information.");
    msgBox.setDetailedText(error);
    msgBox.exec();
}

void  Scraper::findMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor &searchFor)  {

    if (p.isNull()) {
        p=InProgressDialog::create();
    }

    internalFindMovieInfo(manager,  mediaMovieSearchPtr, searchFor, "fr");
}

void  Scraper::findEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor)  {

    if (p.isNull()) {
        p=InProgressDialog::create();
    }

    internalFindEpisodeInfo(manager, mediaTVSearchPtr, searchFor, "fr");
}


void Scraper::closeDialog(){
    if (p!=nullptr){
        p->close();
        p->deleteLater();
        p = nullptr;
        QCoreApplication::processEvents();
    }
}

