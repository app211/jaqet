#include "jaqetmainwindow.h"
#include "src/mainwindow.h"

#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QGridLayout>

#include "inprogressdialog.h"

JaqetMainWindow* JaqetMainWindow::instance;

JaqetMainWindow::JaqetMainWindow(QWidget *parent) : QMainWindow(parent)
{
    MainWindow* w = new MainWindow(this);

    setCentralWidget(w);
}

JaqetMainWindow::~JaqetMainWindow()
{  
}

JaqetMainWindow* JaqetMainWindow::getInstance()
{
    if(!instance)
    {
        instance = new JaqetMainWindow();
    }
    return instance;
}


QPointer<InProgressDialog> p;

void JaqetMainWindow::showWaitDialog(){
        if (p.isNull()) {
            p=InProgressDialog::create();
        }
}

void JaqetMainWindow::hideWaitDialog(){
    if (!p.isNull()){
        p->close();
        p->deleteLater();
        p.clear();
        QCoreApplication::processEvents();
    }
}

