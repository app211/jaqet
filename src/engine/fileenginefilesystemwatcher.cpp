#include "fileenginefilesystemwatcher.h"


FileEngineFileSystemWatcher::FileEngineFileSystemWatcher() {

}

void FileEngineFileSystemWatcher::setPath( const QString & path ){
    if (m_active){
        if (!m_path.isEmpty()){
            this->removePath(m_path);
        }
         this->addPath(path);
    }
    m_path=path;
}

void FileEngineFileSystemWatcher::disabled(){
    if (m_active) {
        if (!m_path.isEmpty()){
            this->removePath(m_path);
        }
        m_active=false;
    }
}

void FileEngineFileSystemWatcher::enabled(){
    if (!m_active) {
        m_active=true;
        if(!m_path.isEmpty()){
            this->addPath(m_path);
        }
    }
}
