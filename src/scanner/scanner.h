#ifndef SCANNER_H
#define SCANNER_H

#include <QString>
#include <QFileInfo>
#include "mediainfo.h"

class Scanner
{

public:
    Scanner();

    struct MediaType {
        MediaType()
            : season( -1 ),
              episode( -1 ) {
        }

        bool isValidForTV() const {
            return( !name.isEmpty() && season >= 0 && episode >= 0 );
        }

        bool isEmpty(){
            return name.isEmpty();
        }

        QString name;
        int season;
        int episode;
        int year;
    };

    struct AnalysisResult {
        MediaType mediaType;
        MediaInfo mediaInfo;

        AnalysisResult(int year){
            mediaType.year=year;
        }

        AnalysisResult(){
         }
    };

    virtual AnalysisResult analyze( const QFileInfo& fi ) const=0 ;
};

#endif // SCANNER_H
