#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QWidget>
#include <QAbstractButton>
#include "scrapers/scraper.h"

class QStyleOptionButton;
class MediaChooserWidget;

class MyButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit MyButton(QWidget *parent = 0);
    void showPopup();
    void setPopup(MediaChooserWidget *popup, Scraper::ImageType filter);

protected:
    virtual void paintEvent(QPaintEvent *e);
private:
    void initStyleOption(QStyleOptionButton *option) const;
    MediaChooserWidget* _popup;
    Scraper::ImageType _filter;

signals:

public slots:
private slots:
    void _q_popupPressed();
    void popupClose();
    void myfocusOutEvent ( QFocusEvent * event );
};

#endif // MYBUTTON_H
