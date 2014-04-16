#ifndef MYPROXYMODEL_H
#define MYPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

class MyProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    static quintptr id;

    QFileSystemModel* ff;
    QSortFilterProxyModel* f5;

    QModelIndex  MyCreateIndex(int row, int column, const QModelIndex& parent, const QModelIndex& sourceIndex ) const;
public:
    MyProxyModel(QFileSystemModel*f, QObject *parent = 0);

    QModelIndex setRootPath ( const QString & newPath );
    bool isDir(const QModelIndex &index) const;
    QFileInfo fileInfo(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &index) const ;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    bool hasChildren(const QModelIndex &index) const;
    void setSourceModel(QAbstractItemModel *sourceModel);
    QVariant data ( const QModelIndex & proxyIndex, int role = Qt::DisplayRole ) const;

signals:
    void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);

public slots:
    void _q_sourceDataChanged(const QModelIndex &source_top_left, const QModelIndex &source_bottom_right);
    void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end);
    void _q_sourceAboutToBeReset();
    void _q_sourceReset();
    void _q_sourceLayoutAboutToBeChanged();
    void _q_sourceLayoutChanged();
    void _q_sourceRowsAboutToBeInserted(const QModelIndex &source_parent,
                                        int start, int end);
    void _q_sourceRowsInserted(const QModelIndex &source_parent,
                               int start, int end);
    void _q_sourceRowsAboutToBeRemoved(const QModelIndex &source_parent,
                                       int start, int end);
    void _q_sourceRowsRemoved(const QModelIndex &source_parent,
                              int start, int end);
    void _q_sourceColumnsAboutToBeInserted(const QModelIndex &source_parent,
                                           int start, int end);
    void _q_sourceColumnsInserted(const QModelIndex &source_parent,
                                  int start, int end);
    void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent,
                                          int start, int end);
    void _q_sourceColumnsRemoved(const QModelIndex &source_parent,
                                 int start, int end);
};

#endif // MYPROXYMODEL_H
