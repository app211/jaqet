#ifndef JAQETMAINWINDOW_H
#define JAQETMAINWINDOW_H

#include <QMainWindow>

class InProgressDialog;

class JaqetMainWindow : public QMainWindow
{
    Q_OBJECT

    static JaqetMainWindow* instance;

public:
    explicit JaqetMainWindow(QWidget *parent = 0);
    ~JaqetMainWindow();

    QPointer<InProgressDialog> showWaitDialog();
    void hideWaitDialog();

    static JaqetMainWindow* getInstance();

signals:

public slots:
};

#endif // JAQETMAINWINDOW_H
