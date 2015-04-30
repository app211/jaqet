#ifndef MEDIACHOOSERMEDIATYPE
#define MEDIACHOOSERMEDIATYPE

#include <QLoggingCategory>

enum class ImageType {
    None = 0,
    Banner = 1,
    Poster = 2,
    Backdrop = 4,
    Thumbnail = 8,
    All=Banner | Poster | Backdrop | Thumbnail
};

Q_DECLARE_LOGGING_CATEGORY(mediaChooser)

#endif // MEDIACHOOSERMEDIATYPE

