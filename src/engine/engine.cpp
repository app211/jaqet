#include "engine.h"
#include <QDebug>
#include "template/template.h"

Engine::Engine(QObject *parent, const QString& path)
    : QAbstractListModel(parent){
    bingo.addFile(":/resources/images/bingo16x16.png",QSize(16,16));
    bingo.addFile(":/resources/images/bingo32x32.png",QSize(32,32));

}

void Engine::doubleClicked ( const QModelIndex & index ){
    internalDoubleClicked(  index );
}
