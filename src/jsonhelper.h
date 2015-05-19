#ifndef JSONHELPER
#define JSONHELPER

#include <QJsonValue>

namespace JSonHelper {
    inline bool isInt(const QJsonValueRef& val){
        return val.isDouble() && val.toDouble()==val.toInt();
    }
}

#endif // JSONHELPER

