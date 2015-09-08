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


#include "jaqetmainwindow.h"

void Scraper::searchFilm( QNetworkAccessManager* manager, const QString& toSearch, int year, const QString& language) const {

    JaqetMainWindow::getInstance()->showWaitDialog();

    internalSearchFilm( manager, toSearch, language, year);
}

void Scraper::searchTV( QNetworkAccessManager* manager, const QString& toSearch, const QString& language)  {

    JaqetMainWindow::getInstance()->showWaitDialog();

    internalSearchTV( manager, toSearch, language);
}


void  Scraper::findMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor &searchFor, const QString& language)  {

    JaqetMainWindow::getInstance()->showWaitDialog();

    internalFindMovieInfo(manager,  mediaMovieSearchPtr, searchFor, language);
}

void  Scraper::findEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language)  {

    JaqetMainWindow::getInstance()->showWaitDialog();

    internalFindEpisodeInfo(manager, mediaTVSearchPtr, searchFor, language);
}

void Scraper::showErrorDialog(const QString& error){
    closeDialog();

    QMessageBox msgBox;
    msgBox.setText("Unable to retrieve information.");
    msgBox.setDetailedText(error);
    msgBox.exec();
}

void Scraper::closeDialog(){
    JaqetMainWindow::getInstance()->hideWaitDialog();
}

