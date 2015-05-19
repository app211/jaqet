#ifndef JAQETMAINWINDOW_H
#define JAQETMAINWINDOW_H

#include <QMainWindow>

class QLightBoxWidget;

class JaqetMainWindow : public QMainWindow
{
    Q_OBJECT

    static JaqetMainWindow* instance;

    QLightBoxWidget* lightBox;

    void showLightBox();
    void hideLightBox();

public:
    explicit JaqetMainWindow(QWidget *parent = 0);
    ~JaqetMainWindow();

    void showWaitDialog();
    void hideWaitDialog();

    static JaqetMainWindow* getInstance();

signals:

public slots:
};

#endif // JAQETMAINWINDOW_H
