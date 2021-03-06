#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QAction>
#include <QGraphicsScene>
#include <QStringListModel>
#include <QPixmap>


class MyProxyModel;
class Engine;
#include <QNetworkAccessManager>

namespace Ui {
class MainWindow;
}

class FileDownloader;


#include "scrapers/scraper.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:

private slots:

    void currentChanged ( const QModelIndex & current, const QModelIndex & previous );

    void doubleClicked (const QModelIndex &);

    void modelReset();
    void modelAboutToBeReset();


    void on_toolButtonExit_clicked();


    void on_toolButtonChooseFolder_clicked();


    void on_toolButtonSettings_clicked();

private:
    MyProxyModel* model;
    QAction* allocineAction;
    QAction* tmdbAction;
    QAction* tvdbAction;

    QGraphicsScene* scene;
    Engine* modelB;
    Ui::MainWindow *ui;
    FileDownloader* m_pImgCtrl;
    QStringList keywordsList;
    QFileInfo fileInfo;

    QMap<QString,QString> _texts;

    QNetworkAccessManager manager;

    QList<Scraper*> scrapes;


};


#endif // MAINWINDOW_H
