#ifndef VECTORLISTMODEL_H
#define VECTORLISTMODEL_H

#include <QAbstractTableModel>
#include "globalHeader.h"

class vectorListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit vectorListModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    double data(int row, int col);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void setPointer(ParameterData * currPar);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setData(int row, int col, double data);
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & /*index*/) const;
    void emitDataChanged();
    /*!
     * \brief setAllData
     * \param inList
     * Add the batch data to the model
     */
    void setAllData(QVector < QVector < double > > inList) {
        list = inList;
    }
    void setAllDst(QVector < ParameterData * > inDsts) {
        destinations = inDsts;
    }

    void moveIndex(int row);

private:
    QVector < QVector < double > > list;
    QVector < ParameterData * > destinations;

signals:

public slots:

};

#endif // VECTORLISTMODEL_H
