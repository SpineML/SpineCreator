/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use GUI for             **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013-2014 Alex Cope, Paul Richmond, Seb James           **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Alex Cope                                            **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#include "vectormodel.h"
//#include "stringify.h"

vectorModel::vectorModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    this->currPar = (ParameterData *)0;
}

int vectorModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return currPar->value.size()+1;
}
int vectorModel::columnCount(const QModelIndex &/*parent = QModelIndex()*/) const
{
    return 2;
}
QVariant vectorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0) {
            if (index.row() < (int) currPar->indices.size())
                return currPar->indices[index.row()];
            else if (index.row() == (int) currPar->indices.size())
                return "";
            else
                cerr << "oops, trying to read out of range value from a vector";
        }
        else if (index.column() == 1) {
            if (index.row() < (int) currPar->value.size())
                return currPar->value[index.row()];
            else if (index.row() == (int) currPar->value.size())
                return "";
            else
                cerr << "oops, trying to read out of range value from a vector";
        }

    }
    return QVariant();
}
void vectorModel::setPointer(ParameterData * currPar)
{
    this->currPar = currPar;
}

void vectorModel::emitDataChanged() {
   QModelIndex index = this->createIndex(0,0);
   emit dataChanged(index, index);
}

QVariant vectorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return "index";
            if (section == 1)
                return this->currPar->name;
        }
        if (orientation == Qt::Vertical) {
            return section;
        }
    }
    //if (role == Qt::)
    return QVariant();
}

bool vectorModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {

        if (index.column() == 0) {
            if (index.row() < (int) currPar->indices.size()) {
                currPar->indices[index.row()] = value.toInt();
                return true;
            } else if (index.row() == (int) currPar->indices.size()) {
                beginInsertRows(this->createIndex(currPar->indices.size()-1, 0).parent(),currPar->indices.size(),currPar->indices.size());
                currPar->value.resize(currPar->value.size()+1, 0);
                currPar->indices.resize(currPar->indices.size()+1, value.toInt());
                endInsertRows();
                emit setSpinBoxVal(currPar->value.size());
            }
        }
        else if (index.column() == 1) {
            if (index.row() < (int) currPar->value.size()) {
                currPar->value[index.row()] = value.toFloat();
                return true;
            } else if (index.row() == (int) currPar->indices.size()) {
                beginInsertRows(this->createIndex(currPar->indices.size()-1, 0).parent(),currPar->indices.size(),currPar->indices.size());
                currPar->value.resize(currPar->value.size()+1, value.toFloat());
                currPar->indices.resize(currPar->indices.size()+1, 0);
                endInsertRows();
                emit setSpinBoxVal(currPar->value.size());
            }
        }
    }
    return false;
}


bool vectorModel::insertRows(int row) {

    if (row > (int) currPar->indices.size()) {
        beginInsertRows(this->createIndex(currPar->indices.size()-1, 0).parent(),currPar->indices.size(),row-1);

        int start = currPar->indices.size();
        currPar->value.resize(row,0);
        currPar->indices.resize(row,0);
        // and fill in indices
        for (uint i = start; i < currPar->indices.size(); ++i)
            currPar->indices[i] = i;

        endInsertRows();
        emit setSpinBoxVal(currPar->value.size());
    }
    if (row < (int) currPar->indices.size()) {
        beginRemoveRows(this->createIndex(currPar->indices.size()-1, 0).parent(),row, currPar->indices.size()-1);
        currPar->value.resize(row,0);
        currPar->indices.resize(row,0);

        endRemoveRows();
        emit setSpinBoxVal(currPar->value.size());
    }

    return true;

}


Qt::ItemFlags vectorModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}
