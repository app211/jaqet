#ifndef CHECKLOCK_H
#define CHECKLOCK_H

#include <QCheckBox>

class CheckLock : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckLock(QWidget *parent = 0);
    ~CheckLock();

    bool isLock() const;
signals:

public slots:
};

#endif // CHECKLOCK_H
