#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QAction>
#include <QGraphicsScene>
#include <QStringListModel>
#include <QPixmap>
#include "template/templateyadis.h"



class MyProxyModel;
class MyQStringListModel;
#include <QNetworkAccessManager>

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
    void setMovieInfo( const SearchMovieInfo&);
     void setPoster (const QString& url, Scraper* scrape );
    void setBackdrop(const QString& url, Scraper *scrape);
    void buildTvix();

    void savePix();
void s_clicked_texte(QPixmap result);

private:
    MyProxyModel* model;
    QAction* allocineAction;
    QAction* tmdbAction;
    QAction* tvdbAction;

    Scraper* currentScraper;
    QGraphicsScene* scene;
    MyQStringListModel* modelB;
    Ui::MainWindow *ui;
    FileDownloader* m_pImgCtrl;
    QStringList keywordsList;
    QFileInfo fileInfo;

    ScraperResource _poster;
    ScraperResource _backdrop;
    QMap<QString,QString> _texts;


    QString _episodeTV;
    QString _seasonTV;
    bool _tvShowTV=false;

    QNetworkAccessManager manager;
    templateYadis b;

    void setImageFromInternet(QByteArray &qb, QGraphicsPixmapItem* itemToUpdate, int x, int y, int w, int h);

};


#endif // MAINWINDOW_H
