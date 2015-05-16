#ifndef JAQETMAINWINDOW_H
#define JAQETMAINWINDOW_H

#include <QMainWindow>

class QLightBoxWidget;

class JaqetMainWindow : public QMainWindow
{
    Q_OBJECT

    static JaqetMainWindow* instance;

    QLightBoxWidget* lightBox;

public:
    explicit JaqetMainWindow(QWidget *parent = 0);
    ~JaqetMainWindow();

    void showLightBox();

    void hideLightBox();

    static JaqetMainWindow* getInstance()
   {
     if(!instance)
     {
       instance = new JaqetMainWindow();
     }
     return instance;
   }
signals:

public slots:
};

#endif // JAQETMAINWINDOW_H
