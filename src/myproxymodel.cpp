#include "myproxymodel.h"
#include <QDebug>
#include <QMap>
#include <QList>

MyProxyModel::MyProxyModel(QFileSystemModel *f, QObject *parent) :
    QAbstractProxyModel(parent)
{
    ff= f;

    setSourceModel(f);

}

QModelIndex MyProxyModel::setRootPath ( const QString & newPath ){
    QModelIndex j=ff->setRootPath(newPath);
    QModelIndex result = mapFromSource(j);
    return result;
}

void MyProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{

    beginResetModel();

    /*   disconnect(this->sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(_q_sourceDataChanged(QModelIndex,QModelIndex)));
*/
    QAbstractProxyModel::setSourceModel(sourceModel);

    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(_q_sourceDataChanged(QModelIndex,QModelIndex)));
    connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(_q_sourceHeaderDataChanged(Qt::Orientation,int,int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(_q_sourceLayoutAboutToBeChanged()));
    connect(sourceModel, SIGNAL(layoutChanged()),
            this, SLOT(_q_sourceLayoutChanged()));
    connect(sourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(_q_sourceAboutToBeReset()));
    connect(sourceModel, SIGNAL(modelReset()), this, SLOT(_q_sourceReset()));

    endResetModel();
}

void MyProxyModel::_q_sourceDataChanged(const QModelIndex &source_top_left, const QModelIndex &source_bottom_right){
    emit dataChanged (mapFromSource(source_top_left), mapFromSource( source_bottom_right) );
}

bool MyProxyModel::isDir(const QModelIndex &index) const{
    return ff->isDir(mapToSource(index));
}

QFileInfo MyProxyModel::fileInfo(const QModelIndex &index) const{
    return ff->fileInfo(mapToSource(index));
}


int MyProxyModel::rowCount(const QModelIndex &parent) const {

    if (parent.isValid() && mapToSource(parent).isValid() ){
        return sourceModel()->rowCount(mapToSource(parent))+1;
    }

    return  0;
}


int MyProxyModel::columnCount(const QModelIndex &parent) const {

    if (parent.isValid() && mapToSource(parent).isValid() ){
        return sourceModel()->columnCount(mapToSource(parent));
    }

    return  0;
}

quintptr MyProxyModel::id=0;

QMap<QModelIndex,QModelIndex> mappingToSource;
QMap<QModelIndex,QModelIndex> mappingFromSource;
QMap<QModelIndex,QModelIndex> mappingParent;
QMap<QModelIndex,QList<QModelIndex> > mappingChild;

QModelIndex  MyProxyModel::MyCreateIndex(int row, int column, const QModelIndex& parent, const QModelIndex& sourceIndex ) const {

    if (!sourceIndex.isValid()){
        return QModelIndex();
    }

    QModelIndex result = createIndex(row, column,id++);
    mappingFromSource[sourceIndex]=result;
    mappingToSource[result]=sourceIndex;

    mappingParent[result]=parent;
    if (!mappingChild.contains(parent)){
        mappingChild[parent]=QList<QModelIndex>();

    }
    mappingChild[parent].append(result);
}

QVariant MyProxyModel::data ( const QModelIndex & proxyIndex, int role ) const {
    qDebug() << proxyIndex;
      QVariant v;
    if (proxyIndex.isValid() && proxyIndex.row()==0){
        if (role==0){
        v= QVariant(QString("..."));
        } else if (role==1){
            v= QVariant(QIcon());
        }else {
            v= QVariant();
        }
    } else {
        v= QAbstractProxyModel::data ( proxyIndex,  role  );

    }
  qDebug() << proxyIndex << role << v;
    return v;
}

QModelIndex MyProxyModel::index(int row, int column, const QModelIndex& parent ) const {

    if (row < 0 || column < 0)
        return QModelIndex();

    if (row>0){
        if (mappingChild.contains(parent)){
            foreach (const QModelIndex& child, mappingChild[parent] ){
                if (child.column() == column && child.row()==row){
                    return child;
                }
            }
        }

        QModelIndex sourceIndex = sourceModel()->index(row,column,mapToSource(parent));

        return MyCreateIndex(row,column,parent,sourceIndex);
    }

    return createIndex(row, column,id++);
}

// parent is at -1, -1. All others have a parent of -1, -1
QModelIndex MyProxyModel::parent(const QModelIndex &index) const {

    if (index.isValid()){
        return mapFromSource(sourceModel()->parent(mapToSource(index)));
    }

    return QModelIndex();
}


QModelIndex MyProxyModel::mapToSource(const QModelIndex &index) const {
    if (index.isValid()){


        if (!mappingToSource.contains(index)){
            QModelIndex sourceIndex;
            if (index.row()>0){
                if (mappingParent.contains(index)){
                    sourceIndex= sourceModel()->index(index.row()-1,index.column(), mappingParent[index]);
                } else {
                    sourceIndex= sourceModel()->index(index.row()-1,index.column());
                }
            }
            mappingToSource[index]=sourceIndex;
            mappingFromSource[sourceIndex]=index;
        }

        return mappingToSource.value(index);
    }

    return QModelIndex();
}


QModelIndex MyProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
    if (sourceIndex.isValid()){
        if (!mappingFromSource.contains(sourceIndex)){
            QModelIndex sourceParent=sourceModel()->parent(sourceIndex);
            QModelIndex parent=mapFromSource(sourceParent);
            MyCreateIndex(sourceIndex.row()+1, sourceIndex.column(),parent, sourceIndex);

        }

        return mappingFromSource.value(sourceIndex);
    }

    return QModelIndex();
}


// does this index have children?
bool MyProxyModel::hasChildren(const QModelIndex &index) const {
    return sourceModel()->hasChildren(mapToSource(index));
}
void MyProxyModel::_q_sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end) {
    qDebug() << "_q_sourceHeaderDataChanged";
}

void MyProxyModel::_q_sourceAboutToBeReset(){
    qDebug() << "_q_sourceAboutToBeReset";
}
void MyProxyModel::_q_sourceReset(){
    qDebug() << "_q_sourceReset";
}
void MyProxyModel::_q_sourceLayoutAboutToBeChanged(){
    qDebug() << "_q_sourceLayoutAboutToBeChanged";
    emit layoutAboutToBeChanged();
}
void MyProxyModel::_q_sourceLayoutChanged(){
    qDebug() << "_q_sourceLayoutChanged";
   emit layoutChanged();
}


void MyProxyModel::_q_sourceRowsAboutToBeInserted(const QModelIndex &source_parent,
                                                  int start, int end){
    qDebug() << "_q_sourceRowsAboutToBeInserted";


 //   emit rowsAboutToBeInserted(mapFromSource(source_parent), start+1, end+1);
}
void MyProxyModel::_q_sourceRowsInserted(const QModelIndex &source_parent,
                                         int start, int end){
    qDebug() << "_q_sourceRowsInserted";
  //  emit rowsInserted(mapFromSource(source_parent), start+1, end+1);
}
void MyProxyModel::_q_sourceRowsAboutToBeRemoved(const QModelIndex &source_parent,
                                                 int start, int end){
    qDebug() << "_q_sourceRowsAboutToBeRemoved";
}
void MyProxyModel::_q_sourceRowsRemoved(const QModelIndex &source_parent,
                                        int start, int end){
    qDebug() << "_q_sourceRowsRemoved";
}
void MyProxyModel::_q_sourceColumnsAboutToBeInserted(const QModelIndex &source_parent,
                                                     int start, int end){
    qDebug() << "_q_sourceColumnsAboutToBeInserted";
}
void MyProxyModel::_q_sourceColumnsInserted(const QModelIndex &source_parent,
                                            int start, int end){
    qDebug() << "_q_sourceColumnsInserted";
}
void MyProxyModel::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent,
                                                    int start, int end){
    qDebug() << "_q_sourceColumnsAboutToBeRemoved";
}
void MyProxyModel::_q_sourceColumnsRemoved(const QModelIndex &source_parent,
                                           int start, int end){
    qDebug() << "_q_sourceColumnsRemoved";
}
