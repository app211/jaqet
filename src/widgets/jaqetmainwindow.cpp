#include "jaqetmainwindow.h"
#include "src/mainwindow.h"

#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QGridLayout>

#include "third/QLightBoxWidget/src/qlightboxwidget.h"
#include "inprogressdialog.h"

JaqetMainWindow* JaqetMainWindow::instance;

JaqetMainWindow::JaqetMainWindow(QWidget *parent) : QMainWindow(parent)
{
    MainWindow* w = new MainWindow(this);

    setCentralWidget(w);

    lightBox = new QLightBoxWidget(this);

    QLabel* lbTitle = new QLabel(tr("QLightBoxWidget"));
    lbTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: white");
    QLabel* lbProgress = new QLabel;
    QMovie* progressMovie = new QMovie(":/resources/animations/ajax-loader.gif");
    lbProgress->setMovie(progressMovie);
    progressMovie->start();
    QLabel* lbDescription = new QLabel(tr("Example how to use QLightBoxWidget\n"
                                          "in your QtWidgets applications..."));
    lbDescription->setStyleSheet("color: white");
    QPushButton* lbClose = new QPushButton(tr("Close"));

    QGridLayout* lbLayout = new QGridLayout;
    lbLayout->setRowStretch(0, 1);
    lbLayout->setColumnStretch(0, 1);
    lbLayout->addWidget(lbTitle, 1, 1);
    lbLayout->addWidget(lbProgress, 1, 2, Qt::AlignRight);
    lbLayout->setColumnStretch(3, 1);
    lbLayout->addWidget(lbDescription, 2, 1, 1, 2);
    lbLayout->addWidget(lbClose, 3, 2);
    lbLayout->setRowStretch(4, 1);

    //	connect(showLB, SIGNAL(clicked()), lightBox, SLOT(show()));
    //	connect(lbClose, SIGNAL(clicked()), lightBox, SLOT(hide()));
    lightBox->setLayout(lbLayout);
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

void JaqetMainWindow::showLightBox(){
    lightBox->show();
}

void JaqetMainWindow::hideLightBox(){
    lightBox->hide();
}

QPointer<InProgressDialog> p;

void JaqetMainWindow::showWaitDialog(){
    if (this->hasFocus()){
        showLightBox();
    } else {
        if (p.isNull()) {
            p=InProgressDialog::create();
        }
    }
}

void JaqetMainWindow::hideWaitDialog(){
    if (!p.isNull()){
        p->close();
        p->deleteLater();
        p.clear();
        QCoreApplication::processEvents();
    } else {
        hideLightBox();
    }
}

