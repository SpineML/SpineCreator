/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use, GUI for            **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013 Alex Cope, Paul Richmond                           **
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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef EDITABLETEXTITEM_H
#define EDITABLETEXTITEM_H

#include <QtGui>
#include <QGraphicsScene>
#include "QGraphicsTextItem"

#define TEXT_PADDING 8
#define PADDING 10
#define ROUNDNESS 15

class TextItemGroup;


class SelectableItem
{
public:
    SelectableItem(){}
protected:
    virtual void handleSelection() = 0;
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event ) = 0;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) = 0;
};


class GroupedTextItem : public QGraphicsTextItem, public SelectableItem
{
    Q_OBJECT
public:
    explicit GroupedTextItem(TextItemGroup *parent = 0, QGraphicsScene *scene = 0);
    ~GroupedTextItem();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    int physicalTextWidth();
    virtual void updateContent() = 0;
    void setColour(QColor c);
    void setPlainText (const QString &text); //overwrites QGraphicsTextItem
    TextItemGroup *getTextItemGroup();
    int getIndexPosition();

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent (QKeyEvent * event);

protected:
    QColor colour;

private:
    TextItemGroup* group;
};


class TitleTextItem: public GroupedTextItem{
    Q_OBJECT
public:
    TitleTextItem(TextItemGroup *parent = 0);
    virtual void updateContent();
protected:
    virtual void handleSelection();

};

class TextItemGroup: public QObject, public QGraphicsItem, public SelectableItem{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    TextItemGroup();
    ~TextItemGroup();
    void addMember(GroupedTextItem* m);
    void removeMember(GroupedTextItem* m);
    void deleteAllMembers();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int singleItemHeight();
    void setPadding(int p);
    void setRounded(bool b);
    void setColour(QColor c);
    void setBorderColour(QColor c);
    int getPadding();
    void setSelected(bool selected);  //overwrite qgraphicsitem
    void updateItemDimensions();
    void moveMemberUp(GroupedTextItem* item);
    void moveMemberDown(GroupedTextItem* item);
    int getTextItemIndex(GroupedTextItem *item);
    void updateMemberContents();
    int getMemberCount();
protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
protected :
    TitleTextItem* title;
    QVector<GroupedTextItem*> members;
    int padding;
    bool rounded;
    QColor colour;
    QColor border_colour;
};




#endif // EDITABLETEXTITEM_H
