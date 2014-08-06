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

#include "connectionmodel.h"

csv_connectionModel::csv_connectionModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    this->currentConnection = (csv_connection *)0;
}

 int csv_connectionModel::rowCount(const QModelIndex & /*parent*/) const
 {
     if (!(this->currentConnection == (csv_connection *)0)) {
         return this->currentConnection->getNumRows() + 1;
     }
     return 0;
 }

 int csv_connectionModel::columnCount(const QModelIndex & /*parent*/) const
 {
     if (!(this->currentConnection == (csv_connection *)0)) {
         return this->currentConnection->getNumCols();
     }
     return 0;
 }

 QVariant csv_connectionModel::data(const QModelIndex &index, int role) const
 {
     if (role == Qt::DisplayRole)
     {
        QModelIndex tempInd = index;
        if (index.row() == currentConnection->getNumRows())
            return "";
        else {
            float out = this->currentConnection->getData(tempInd);
            return out;}

     }
     return QVariant();
 }

 void csv_connectionModel::allData(QVector < conn > &conns) {

    // read all data into memory for displaying connection lists:
    this->currentConnection->getAllData(conns);

 }

 void csv_connectionModel::setConnection(csv_connection * currConn) {
     this->currentConnection = currConn;
 }

 csv_connection * csv_connectionModel::getConnection() {
     return this->currentConnection;
 }

 QVariant csv_connectionModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role == Qt::DisplayRole)
     {
         if (orientation == Qt::Horizontal) {
             return this->currentConnection->getHeader(section);
         }
         if (orientation == Qt::Vertical) {
             return section;
         }
     }
     //if (role == Qt::)
     return QVariant();
 }

 bool csv_connectionModel::setData(const QModelIndex & index, const QVariant & value, int role)
 {
     if (role == Qt::EditRole)
     {
         if (index.row() == currentConnection->getNumRows()) {
                beginInsertRows(this->createIndex(currentConnection->getNumRows()-1, 0).parent(),currentConnection->getNumRows(),currentConnection->getNumRows());
                    currentConnection->setNumRows(currentConnection->getNumRows()+1);
                    for (int i = 0; i < currentConnection->getNumCols(); ++i) {
                        currentConnection->setData(this->createIndex(currentConnection->getNumRows()-1,i), 0);
                    }
                endInsertRows();
                setSpinBoxVal(currentConnection->getNumRows());
         }
         //save value from editor
         this->currentConnection->setData(index, value.toFloat());
         //for presentation purposes only: build and emit a joined string
         QString result = value.toString();
         emit editCompleted( result );

     }
     emit dataChanged(index, index);

     return true;
 }

 void csv_connectionModel::emitDataChanged() {
    QModelIndex index = this->createIndex(0,0);
    emit dataChanged(index, index);
 }

 Qt::ItemFlags csv_connectionModel::flags(const QModelIndex & /*index*/) const
 {
     return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
 }

 bool csv_connectionModel::insertConnRows(int row) {

     if (row > currentConnection->getNumRows()) {
         beginInsertRows(this->createIndex(currentConnection->getNumRows()-1, 0).parent(),currentConnection->getNumRows(),row-1);

         int start = currentConnection->getNumRows();
         currentConnection->setNumRows(row);
         // and fill in extra rows
         for (int i = start; i < currentConnection->getNumRows(); ++i)
             for (int j = 0; j < currentConnection->getNumCols(); ++j)
                currentConnection->setData(this->createIndex(i,j), 0);

         endInsertRows();
         setSpinBoxVal(currentConnection->getNumRows());
     }
     if (row < currentConnection->getNumRows()) {
         beginRemoveRows(this->createIndex(currentConnection->getNumRows()-1, 0).parent(),row, currentConnection->getNumRows()-1);

         currentConnection->setNumRows(row);

         endRemoveRows();
         setSpinBoxVal(currentConnection->getNumRows());
     }

     return true;

 }
