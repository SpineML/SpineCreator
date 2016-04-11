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

#ifndef SYSTEMMODEL_H
#define SYSTEMMODEL_H

#include <QAbstractItemModel>
#include "globalHeader.h"
#include "NL_systemobject.h"

class TreeItem;

class systemmodel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit systemmodel(nl_rootdata * dataPtr, QObject *parent = 0);
    ~systemmodel();
    nl_rootdata * dataPtr;
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & /*index*/) const;
    void emitDataChanged();
    QModelIndex parent( const QModelIndex & index ) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndexList getPersistentIndexList();

private:
    void setupModelData(TreeItem *parent);
    TreeItem *rootItem;

signals:
    
public slots:
    
};

class TreeItem
{
public:
    TreeItem(const QList<QVariant> &data, bool * check, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();
    bool isChecked() const { return *checked; }
    void setChecked( bool set ) { *checked = set; }
    systemObjectType type;
    QString name;

private:
    QList<TreeItem*> childItems;
    QList<QVariant> itemData;
    TreeItem *parentItem;
    bool * checked;
};


#endif // SYSTEMMODEL_H
