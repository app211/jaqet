#ifndef FILEENGINE_H
#define FILEENGINE_H

#include "engine.h"

#include <QDir>
#include <QFileIconProvider>


class FileEngine : public Engine
{
    Q_OBJECT
public:
    explicit FileEngine(QObject *parent = 0);
    void cdUp();
    void cd(const QString& path);
    QFileInfo fileInfo(const QModelIndex & index)const;

    Engine::TypeItem getTypeItem(const QModelIndex & index) const;
    virtual Engine::TypeItem getTypeItem( const QFileInfo & f) const=0;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool canGiveFileInfo() final {return true;}
    QFileInfo getFileInfo (const QModelIndex &index) final {return fileInfo(index);}
    void internalDoubleClicked ( const QModelIndex & index );

protected:
    void populate();

private:
    QFileInfoList entryInfoList;
    bool allowUp=false;
    QFileIconProvider iconProvider;
    QDir currentDir;

signals:

public slots:

};

#endif // FILEENGINE_H
