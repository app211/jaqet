#ifndef FILEENGINEFILESYSTEMWATCHER_H
#define FILEENGINEFILESYSTEMWATCHER_H

#include <QFileSystemWatcher>

class FileEngineFileSystemWatcher : public QFileSystemWatcher {
    Q_OBJECT

    QString m_path;
    bool m_active=true;
public :
    FileEngineFileSystemWatcher() ;

    void setPath( const QString & path );
    void disabled();
    void enabled();
};

#endif // FILEENGINEFILESYSTEMWATCHER_H
