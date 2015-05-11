#include "checklock.h"
#include <QVariant>

CheckLock::CheckLock(QWidget *parent) : QCheckBox(parent)
{
   setProperty("CheckLock", QVariant::fromValue(true));

}

CheckLock::~CheckLock()
{

}

 bool CheckLock::isLock() const {
     return !isChecked();
 }
