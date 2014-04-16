#ifndef AVPROBE_H
#define AVPROBE_H

#include "av.h"

struct avprobe_version {
    bool isOk;
    QString m_major;
    QString m_minor;

};
class avprobe
{
public:
    avprobe();

   avprobe_version getVersion(const QString& unrarPath);
   void getInfo(const QString& filePath);
};

#endif // AVPROBE_H
