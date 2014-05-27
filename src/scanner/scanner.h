#ifndef SCANNER_H
#define SCANNER_H

#include <QString>
#include <QFileInfo>

class Scanner
{

public:
    Scanner();

    struct AnalysisResult {
        AnalysisResult()
            : season( -1 ),
              episode( -1 ) {
        }

        bool isValidForTV() const {
            return( !name.isEmpty() && season >= 0 && episode >= 0 );
        }

        QString name;
        int season;
        int episode;
    };

    virtual AnalysisResult analyze( const QFileInfo& fi ) const=0 ;
};

#endif // SCANNER_H
