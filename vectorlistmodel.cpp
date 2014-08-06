#include "vectorlistmodel.h"
#include "nineML_classes.h"

vectorListModel::vectorListModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int vectorListModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    if (this->list.size() > 0) {
        return this->list[0].size();
    }
    return 0;
}

int vectorListModel::columnCount(const QModelIndex &/*parent = QModelIndex()*/) const
{
    return this->list.size();
}

double vectorListModel::data(int row, int col) {

    return this->data(this->createIndex(row,col), Qt::DisplayRole).toFloat();

}

QVariant vectorListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.column() > list.size()-1) {
            return "#Err";
        }
        if (index.row() > list[index.column()].size()-1) {
            return "#Err";
        }
        return list[index.column()][index.row()];

    }
    return QVariant();
}

void vectorListModel::emitDataChanged() {
   QModelIndex index = this->createIndex(0,0);
   emit dataChanged(index, index);
}

QVariant vectorListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            if (section > this->destinations.size()-1) {
                return "#Err";
            }
            if (this->destinations[section] == NULL) {
                return "#ErrNull";
            }
            return this->destinations[section]->name;
        }
        if (orientation == Qt::Vertical) {
            return section;
        }
    }
    //if (role == Qt::)
    return QVariant();
}

void vectorListModel::setData(int row, int col, double data) {

    // create and index and prod the real method
    QModelIndex index = this->createIndex(row,col);
    this->setData(index, data, Qt::EditRole);

}

bool vectorListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {

        if (index.column() < list.size()) {

            if (index.row() < list[index.column()].size()) {
                // modify existing experimental set
                list[index.column()][index.row()] = value.toDouble();
                return true;
            } else if (index.row() == list[index.column()].size()) {
                // add a new experimental set
                beginInsertRows(this->createIndex(list[index.column()].size()-1, 0).parent(),list[index.column()].size(),list[index.column()].size());
                // add extra indices onto all the lists
                for (int i = 0; i < list.size(); ++i) {
                    list[i].push_back(0);
                }
                endInsertRows();
                return true;
            }
        }

    }
    return false;
}

void vectorListModel::moveIndex(int row) {

    // copy the corresponding data row to the experiment
    if (this->list.size() == 0) {
        return;
    }
    if (row < this->list[0].size()) {
        for (int i = 0; i < this->list.size(); ++i) {
            // copy
            this->destinations[i]->value[0] = this->list[i][row];
        }
    }

}


Qt::ItemFlags vectorListModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}
