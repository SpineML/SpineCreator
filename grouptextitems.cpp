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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#include <QtGlobal>
#include <algorithm>

#include "grouptextitems.h"
#include "regimegraphicsitem.h"


GroupedTextItem::GroupedTextItem(TextItemGroup *parent)
    : QGraphicsTextItem(parent)
{
    group = parent;
    setFlag(QGraphicsItem::ItemIsSelectable);
    colour = Qt::white;
    setDefaultTextColor(Qt::black);
    QTextOption option = document()->defaultTextOption ();
    option.setAlignment (Qt::AlignCenter);
    document()->setDefaultTextOption ( option );
}

GroupedTextItem::~GroupedTextItem()
{

}

void GroupedTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    handleSelection();
    QGraphicsTextItem::mousePressEvent(event);
}

void GroupedTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    //ignore: uncomment for direct editing
    //if (textInteractionFlags() == Qt::NoTextInteraction)
    //    setTextInteractionFlags(Qt::TextEditorInteraction);
    //QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void GroupedTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF text_rect = boundingRect();
    painter->setPen(colour);
    painter->setBrush(colour);
    painter->drawRect(text_rect);
    QGraphicsTextItem::paint(painter, option, widget);
}


void GroupedTextItem::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return)||(event->key() == Qt::Key_Enter)){
        clearFocus();
        setSelected(false);
    }else
        QGraphicsTextItem::keyPressEvent(event);

}

QRectF GroupedTextItem::boundingRect() const
{
    QRectF b = QGraphicsTextItem::boundingRect();
    b.adjust(-TEXT_PADDING, -TEXT_PADDING, TEXT_PADDING, TEXT_PADDING);
    return b;
}


int GroupedTextItem::physicalTextWidth()
{
    QFontMetricsF fm(this->font());
    return fm.width(toPlainText())+20;
}

void GroupedTextItem::setColour(QColor c)
{
    colour = c;
}

void GroupedTextItem::setPlainText(const QString &text)
{
    QGraphicsTextItem::setPlainText(text);
    group->updateItemDimensions();
}

TextItemGroup * GroupedTextItem::getTextItemGroup()
{
    if (parentItem() != NULL)
    {
        //shouold never be anything but a TextItemGroup
        TextItemGroup *tig = (TextItemGroup*)parentItem();
        return tig;
    }
    else
    {
        qDebug() << "Grouped Text Item has no parent???";
        return NULL;
    }
}

int GroupedTextItem::getIndexPosition()
{
    return group->getTextItemIndex(this);
}






/************************************************************************************/

TextItemGroup::TextItemGroup()
: QGraphicsItem()
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    title = new TitleTextItem(this);
    padding = PADDING;
    rounded = true;
    colour = Qt::white;
    border_colour = Qt::black;
}

TextItemGroup::~TextItemGroup()
{
    deleteAllMembers();
}

QRectF TextItemGroup::boundingRect() const
{
    QRectF bounds = title->boundingRect();
    for (int i=0; i<members.size();i++)
    {
        bounds.setWidth(qMax(bounds.width(), members[i]->boundingRect().width()));
    }
    bounds.setHeight(bounds.height()*(members.size()+1));
    bounds.adjust(-padding, -padding, padding, padding);

    return bounds;
}

void TextItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    //draw bounds
    QRectF rect = TextItemGroup::boundingRect();
    if (isSelected())
        painter->setPen(Qt::red);
    else
        painter->setPen(border_colour);
    painter->setBrush(colour);
    if (rounded)
        painter->drawRoundRect(rect, 100*ROUNDNESS/rect.width(), 100*ROUNDNESS/rect.height());
    else
        painter->drawRect(rect);
}

void TextItemGroup::updateItemDimensions()
{
    prepareGeometryChange(); //obviously!!

    //iterate child items
    int width_max = title->physicalTextWidth();
    for(int i=0; i<members.size(); i++)
    {
        width_max = qMax(width_max, members[i]->physicalTextWidth());
    }

    //update
    title->setTextWidth(width_max);
    for(int i=0; i<members.size(); i++)
    {
        members[i]->setPos(0, TextItemGroup::singleItemHeight()* (i+1));
        members[i]->setTextWidth(width_max);
    }

    //qDebug() << "TextItemGroup update dims width max is " << width_max;
}

void TextItemGroup::moveMemberUp(GroupedTextItem *item)
{
    int index = std::find(members.begin(), members.end(), item) - members.begin();

    if ((index > 0)&&(index < members.size())){
        std::swap(members[index], members[index-1]);
    }else{
        qDebug() << "Item index " << index << "not found!!";
    }

    updateItemDimensions();
}

void TextItemGroup::moveMemberDown(GroupedTextItem *item)
{
    int index = std::find(members.begin(), members.end(), item) - members.begin();

    if ((index >= 0)&&(index < members.size()-1)){
        std::swap(members[index], members[index+1]);
    }else{
        qDebug() << "Item index " << index << "not found!!";
    }

    updateItemDimensions();
}

int TextItemGroup::getTextItemIndex(GroupedTextItem *item)
{
    return members.indexOf(item);
}

void TextItemGroup::updateMemberContents()
{
    for (int i=0;i<members.size();i++)
        members[i]->updateContent();
}

int TextItemGroup::getMemberCount()
{
    return members.size();
}

void TextItemGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    handleSelection();
    QGraphicsItem::mousePressEvent(event);
}

void TextItemGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    //ignore double clicks
}

void TextItemGroup::addMember(GroupedTextItem *m)
{
    members.push_back(m);
    updateItemDimensions();
}

void TextItemGroup::removeMember(GroupedTextItem *m)
{
    //WTF is this stupid function for? Required to ensure draw calls dont reference thinsg which you delete properly!
    prepareGeometryChange();

    members.erase(std::remove(members.begin(), members.end(), m), members.end());
    //no scene dont need to remove from parent deletion will handle this!
    delete m;
    //m->deleteLater();
    m = NULL;
    updateItemDimensions();
}

void TextItemGroup::deleteAllMembers()
{
    //remove any state assignments
    foreach(GroupedTextItem* member, members)
    {
        removeMember(member);
    }
    if (title != NULL){
        //no scene dont need to remove from parent deletion will handle this!
        delete title;
        //title->deleteLater();
        title = NULL;
    }
}

int TextItemGroup::singleItemHeight()
{
    return title->boundingRect().height();
}



void TextItemGroup::setPadding(int p)
{
    padding = p;
}

void TextItemGroup::setRounded(bool b)
{
    rounded = b;
}

void TextItemGroup::setColour(QColor c)
{
    colour = c;
}

void TextItemGroup::setBorderColour(QColor c)
{
    border_colour = c;
}

/************************************************************/

TitleTextItem::TitleTextItem(TextItemGroup *parent)
    : GroupedTextItem(parent)
{
    colour = Qt::black;
    setDefaultTextColor(Qt::white);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void TitleTextItem::handleSelection(){}
void TitleTextItem::updateContent(){}


void TextItemGroup::setSelected(bool selected)
{
    QGraphicsItem::setSelected(selected);
    if(selected)
        handleSelection();
}







