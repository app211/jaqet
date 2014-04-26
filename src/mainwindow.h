#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QAction>
#include <QGraphicsScene>
#include <QStringListModel>
#include <QPixmap>

class MyQStringListModel : public QStringListModel {
public :

    QDir currentDir;

    explicit MyQStringListModel(QObject *parent = 0, const QString& path=""):
        QStringListModel(parent){
        setPath(path);
    }
    void cdUp(){
        currentDir.cdUp();
        populate();
    }

    void cd(const QString& path){
        if (QFileInfo(currentDir,path).isDir()){
            currentDir.cd(path);
            populate();
        }
    }
    void setPath(const QString& path){
        currentDir =QDir(path);
        populate();
    }

    bool isDir( const QModelIndex & index){

        if (index.isValid()){
             return QFileInfo(currentDir,data(index,Qt::DisplayRole).toString()).isDir();
        }

        return false;
    }

    QFileInfo fileInfo(const QModelIndex & index){

        if (index.isValid()){
             return QFileInfo(currentDir,data(index,Qt::DisplayRole).toString());
        }

        return QFileInfo();
    }

    void populate(){
        currentDir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot);

        setStringList(currentDir.entryList());
    }
    Qt::ItemFlags flags ( const QModelIndex & index ) const{
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
    }
};

class MyProxyModel;

namespace Ui {
class MainWindow;
}

class FileDownloader;


#include "scrapers/scraper.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:

private slots:

    void currentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void searchAllocineMovie();
    void searchScraper();
    void chooseTizzBirdFolder();
    void loadImage();
    void ctxMenu(const QPoint &pos) ;
    void changeFileName();
    void doubleClicked ( const QModelIndex & index );
    void setSynopsis();
    void setPoster (const QString& url, Scraper* scrape );
    void setBackdrop(const QString& url, Scraper *scrape);
    void buildTvix();

    void savePix();

private:
    MyProxyModel* model;
    QAction* allocineAction;
    QAction* tmdbAction;
    Scraper* currentScraper;
    QGraphicsScene* scene;
    MyQStringListModel* modelB;
    Ui::MainWindow *ui;
    FileDownloader* m_pImgCtrl;
    QStringList keywordsList;
    QFileInfo fileInfo;
    ScraperResource _poster;
    ScraperResource _backdrop;
    QString _synopsis;


    QString _episodeTV;
    QString _seasonTV;
    bool _tvShowTV=false;
};

#endif // MAINWINDOW_H
