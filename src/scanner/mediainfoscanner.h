#ifndef MEDIAINFOSCANNER_H
#define MEDIAINFOSCANNER_H

#include "scanner.h"

class MediaInfoScanner : public Scanner
{
public:
    MediaInfoScanner();
    virtual AnalysisResult analyze( const QFileInfo& fi ) const;
};

#endif // MEDIAINFOSCANNER_H
