#ifndef FILENAMESCANNER_H
#define FILENAMESCANNER_H

#include <QList>
#include <QRegExp>
#include <QFileInfo>
#include "scanner.h"

class FileNameScanner : public Scanner
{
public:
    FileNameScanner();
    AnalysisResult analyze( const QFileInfo& fi ) const;

private:
     QList<QRegExp> m_filenameRegExps;
};

#endif // FILENAMESCANNER_H
