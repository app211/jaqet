#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QWidget>
#include <QAbstractButton>
#include <QMouseEvent>

#include "mediachoosermediatype.h"

class QStyleOptionButton;
class MediaChooserPopup;

class MediaChooserButton : public QAbstractButton
{

    Q_OBJECT

public:
    MediaChooserButton(QWidget *parent = 0);
    void showPopup();
    void setPopup(MediaChooserPopup *popup, QFlags<ImageType> filter);
    void setMediaSize(const QSize& mediaSize);
    void setMedia(const QPixmap& media);
    void reset();

protected:
    enum ButtonState
    {
        STATE_NORMAL, STATE_HOVERED, STATE_CLICKED
    };


    ButtonState m_State;

    virtual void paintEvent(QPaintEvent *e);
    virtual  void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual QSize sizeHint() const;
    virtual bool hitButton(const QPoint &pos) const;
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

private:
    void initStyleOption(QStyleOptionButton &option) const;
    MediaChooserPopup* _popup=nullptr;
    QSize _mediaSize=QSize(16,16);
    QPixmap _media;
    QPixmap _mediaScaled;
    QPixmap m_erase;
    QPixmap m_indicator;
    QFlags<ImageType> m_popupFilter;

    QRect eraseIconRect() const;
    QSize getTSize() const;
    bool hitEraseButton(const QPoint &pos) const;

public slots:
    void mediaSelected(const MediaChoosed& mediaChoosed);
    void buttonPressed();
    void popupClosed();

signals:
    void mediaSelected2(const MediaChoosed& mediaChoosed);
  //  void mediaSelected(MediaChoosed mediaChoosed);


};

#endif // MYBUTTON_H
