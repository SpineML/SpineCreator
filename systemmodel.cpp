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

#include "systemmodel.h"
#include "QCheckBox"
#include "rootdata.h"

systemmodel::systemmodel(rootData * dataPtr, QObject *parent) :
    QAbstractItemModel(parent)
{
    this->dataPtr = dataPtr;
    QList<QVariant> rootData;
    rootData << "Model";
    rootItem = new TreeItem(rootData, NULL);
    setupModelData(rootItem);
}

 QModelIndexList systemmodel::getPersistentIndexList()
 {
    return this->persistentIndexList();
 }

 int systemmodel::rowCount(const QModelIndex & parent) const
 {
     TreeItem *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     return parentItem->childCount();
 }

 int systemmodel::columnCount(const QModelIndex & parent) const
 {
     if (parent.isValid())
         return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
     else
         return rootItem->columnCount();
 }

 QVariant systemmodel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

     if ( role == Qt::CheckStateRole && item->type != projectionObject )
         return static_cast< int >( item->isChecked() ? Qt::Checked : Qt::Unchecked );

     if (role != Qt::DisplayRole)
         return QVariant();

     return item->data(index.column());
 }

 bool systemmodel::setData ( const QModelIndex & index, const QVariant & value, int role )
 {
     if (!index.isValid())
         return false;

     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

     if ( role == Qt::CheckStateRole && index.column() == 0 ) {
         item->setChecked(value.toBool());
         // find and set status of item in system:
         for (int i = 0; i < dataPtr->populations.size(); ++i) {

             QSharedPointer <population> currPop = (QSharedPointer <population>) dataPtr->populations[i];

             // populations
             if (currPop->getName() == item->name) {
                 currPop->isVisualised = value.toBool();
             }

             // projections
             for (int j = 0; j < currPop->projections.size(); ++j) {

                 QSharedPointer <projection> currProj = (QSharedPointer <projection>) currPop->projections[j];

                 // synapses
                 for (int k = 0; k < currProj->synapses.size(); ++k) {

                     QSharedPointer <synapse> currTarg = (QSharedPointer <synapse>) currProj->synapses[k];

                     if (currProj->getName() + ": Synapse " + QString::number(k) == item->name) {
                         currTarg->isVisualised = value.toBool();
                     }

                 }

             }

         }
         emit dataChanged(index, index);
         return true;
     }

     return false;
 }

 QModelIndex systemmodel::parent( const QModelIndex & index ) const
 {
     if (!index.isValid())
         return QModelIndex();

     TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
     TreeItem *parentItem = childItem->parent();

     if (parentItem == rootItem)
         return QModelIndex();

     return createIndex(parentItem->row(), 0, parentItem);

 }

 QModelIndex systemmodel::index(int row, int column, const QModelIndex &parent) const
 {
     if (!hasIndex(row, column, parent))
          return QModelIndex();

      TreeItem *parentItem;

      if (!parent.isValid())
          parentItem = rootItem;
      else
          parentItem = static_cast<TreeItem*>(parent.internalPointer());

      TreeItem *childItem = parentItem->child(row);
      if (childItem)
          return createIndex(row, column, childItem);
      else
          return QModelIndex();
 }

QVariant systemmodel::headerData(int section, Qt::Orientation orientation, int role) const
 {

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();

 }

 void systemmodel::emitDataChanged() {
    QModelIndex index = this->createIndex(0,0);
    emit dataChanged(index, index);
 }

 systemmodel::~systemmodel()
 {
     delete rootItem;
 }


Qt::ItemFlags systemmodel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    TreeItem * item = static_cast<TreeItem*>(index.internalPointer());

    if (!(item->type == projectionObject))
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

void systemmodel::setupModelData(TreeItem *parent)
{
    QList<TreeItem*> parents;
    parents << parent;

    for (int pop = 0; pop < dataPtr->populations.size();++pop) {

        // add population
        QSharedPointer <population> currPop = (QSharedPointer <population>) dataPtr->populations[pop];
        QList<QVariant> columnDataPop;
        columnDataPop << currPop->getName();

        // add as child of current parent and set this as the new parent
        parents.last()->appendChild(new TreeItem(columnDataPop, &(currPop->isVisualised), parents.last()));
        parents.last()->child(parents.last()->childCount()-1)->type = currPop->type;
        parents << parents.last()->child(parents.last()->childCount()-1);

        // add generic inputs for Populations

        for (int output = 0; output < dataPtr->populations[pop]->neuronType->outputs.size(); ++output) {

            // add output
            QSharedPointer<genericInput> currOutput = dataPtr->populations[pop]->neuronType->outputs[output];

            // really we can only currently display pop -> pop inputs sensibly...
            if (!currOutput->projInput) {
                if (currOutput->source->type == populationObject && currOutput->destination->type == populationObject) {
                    QList<QVariant> columnDataInput;
                    columnDataInput << "Output from " + currOutput->source->getName() + " to " + currOutput->destination->getName() + " port " + currOutput->dstPort + " " + QString::number(output);

                    // add as child of current parent
                    parents.last()->appendChild(new TreeItem(columnDataInput, &(currOutput->isVisualised), parents.last()));
                    parents.last()->child(parents.last()->childCount()-1)->type = currOutput->type;
                }
            }

        }

        // add projections

        for (int proj = 0; proj < currPop->projections.size(); ++proj) {

            // add projection
            QSharedPointer <projection> currProj = (QSharedPointer <projection>) currPop->projections[proj];
            QList<QVariant> columnDataProj;
            columnDataProj << currProj->getName();

            // add as child of current parent and set this as the new parent
            parents.last()->appendChild(new TreeItem(columnDataProj, NULL, parents.last()));
            parents.last()->child(parents.last()->childCount()-1)->type = currProj->type;
            parents << parents.last()->child(parents.last()->childCount()-1);

            for (int targ = 0; targ < dataPtr->populations[pop]->projections[proj]->synapses.size(); ++targ) {

                // add Synapse
                //QSharedPointer <projection> currTarg = (QSharedPointer <projection>) currProj->synapses[targ];
                QList<QVariant> columnDataTarg;
                columnDataTarg << currProj->getName() + ": Synapse " + QString::number(targ);

                // add as child of current parent
                parents.last()->appendChild(new TreeItem(columnDataTarg, &(currProj->synapses[targ]->isVisualised), parents.last()));
                parents.last()->child(parents.last()->childCount()-1)->type = nullObject;

            }

            // go back a level of parents
            parents.pop_back();

        }

        // go back a level of parents
        parents.pop_back();

    }

}



TreeItem::TreeItem(const QList<QVariant> &data, bool * check, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
    checked = check;
    name = itemData[0].toString();

}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}
