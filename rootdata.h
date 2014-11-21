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
**          Authors: Alex Cope, Seb James                                 **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef ROOTDATA_H
#define ROOTDATA_H

#include "globalHeader.h"

#include "nineml_layout_classes.h"
#include "connection.h"
#include "population.h"
#include "rootlayout.h"
#include "glwidget.h"
#include "connectionmodel.h"
#include "glconnectionwidget.h"
#include "systemobject.h"
#include "valuelistdialog.h"

struct selStruct {
    int type;
    int index;
    int colInd;
};

struct cursorType {
    GLfloat x;
    GLfloat y;
};

struct loadedComponent {
    QSharedPointer<NineMLComponent> component;
    QString url;
};

class rootData : public QObject
{
    Q_OBJECT
public:
    /*!
     * Public methods
     */
    //@{
    explicit rootData(QObject *parent = 0);
    void get_model_xml(QXmlStreamWriter &);
    void get_model_meta_xml(QDomDocument &meta);
    QColor getColor(QColor);
    int getIndex();
    bool selectionMoved;
    void reDrawPanel();
    QSharedPointer<systemObject> getObjectFromName(QString name);
    void callRedrawGLview();
    void updateStatusBar(QString, int);
    void setTitle();
    void replaceComponent(QSharedPointer<NineMLComponent>, QSharedPointer<NineMLComponent>);
    NineMLRootObject* import_component_xml_single(QString fileName);
    bool isComponentInUse(QSharedPointer<NineMLComponent> oldComp);
    bool removeComponent(QSharedPointer<NineMLComponent> oldComp);
    //bool isValidPointer(systemObject *ptr);
    QSharedPointer<systemObject> isValidPointer(systemObject *ptr);
    QSharedPointer<NineMLComponentData> isValidPointer(NineMLComponentData *ptr);
    QSharedPointer<NineMLComponent> isValidPointer(NineMLComponent *ptr);
    void redrawViews();

    /*!
     * Find the object selected by the mouse (called by onLeftMouseDown)
     */
    void findSelection (float xGL, float yGL, float GLscale, QVector <QSharedPointer<systemObject> >& newlySelectedList);
    //@}

public:
    /*!
     * public attributes
     */
    //@{
    QVector < projectObject * > projects;
    projectObject * currProject;

    QVector < QSharedPointer <population> > populations;
    QVector < QSharedPointer<NineMLComponent> > catalogUnsorted;
    QVector < QSharedPointer<NineMLComponent> > catalogNrn;
    QVector < QSharedPointer<NineMLComponent> > catalogWU;
    QVector < QSharedPointer<NineMLComponent> > catalogPS;

    QVector < QSharedPointer<NineMLLayout> > catalogLayout;
    QVector < QString > catalogConn;
    QVector < experiment *> experiments;

    QVector < loadedComponent > loadedComponents;

    // structure to hold selected items
    QVector <QSharedPointer<systemObject> > selList;

    cursorType cursor;
    int largestIndex;
    QImage popImage;
    QSharedPointer<NineMLComponentData> clipboardCData;
    QVector <QSharedPointer<systemObject> > clipboardObjects;
    versionControl* version;
    MainWindow* main;
    QActionGroup* projectActions;
    //@}

signals:
    void undoRenameBox();
    void finishDrawingSynapse();
    void statusBarUpdate(QString, int);
    void updatePanel(rootData *);
    void updatePanelView2(QString);
    void redrawGLview();
    void setCaption(QString);
    void setWindowTitle();
    void itemsSelected(QString);

public slots:
    void saveImage(QString);
    void reDrawAll(QPainter *, float, float, float, int, int, drawStyle style);
    void onLeftMouseDown(float xGL, float yGL, float GLscale, bool shiftDown);

    /*
     * \brief React to an item having been moved
     *
     * The itemWasMoved slot will accept a GLWidget::itemWasMoved
     * signal which says "an item in the user interface was moved"
     *
     * This will then check the type of the object which was moved and
     * react accordingly - that means putting a new entry on the
     * undostack.
     */
    void itemWasMoved();

    void onRightMouseDown(float xGL, float yGL, float GLscale);
    void mouseMoveGL(float, float);
    void updatePortMap(QString);
    void updateComponentType(int index);
    void updatePar();
    void updatePar(int);
    void updateLayoutPar();
    void setSize();
    void setLoc3();
    void addPopulation();
    void addSpikeSource();
    void renamePopulation();
    void startAddBezier(float, float);
    void addBezierOrProjection(float, float);
    void deleteCurrentSelection();
    void changeSynapse();
    void selectColour();
    void getNeuronLocationsSrc(QVector < QVector <loc> >*, QVector <QColor> *, QString name);
#ifdef NEED_MOUSE_UP_LOGIC
    void onLeftMouseUp(float xGL, float yGL, float GLscale);
#endif
    void setSelectionbyName(QString);
    void addgenericInput();
    void delgenericInput();
    void editConnections();
    void dragSelect(float xGL, float yGL);
    void endDragSelection();
    void setCaptionOut(QString);
    void setModelTitle(QString);
    void undoOrRedoPerformed(int);
    void abortProjection();
    void updatePanelView2Accessor();
    /*!
     * \brief copyParsToClipboard
     * Copy the Properties of the selected object to a temporary 'Clipboard'
     */
    void copyParsToClipboard();
    /*!
     * \brief pasteParsFromClipboard
     * Paste the Properties from teh 'Clipboard' to the current selected object
     */
    void pasteParsFromClipboard();
    /*!
     * \brief copySelectionToClipboard
     * Copy the selected objects to a 'Clipboard' and update pointers
     */
    void copySelectionToClipboard();
    /*!
     * \brief pasteSelectionFromClipboard
     * Paste the 'Clipboard' back at the cursor location
     */
    void pasteSelectionFromClipboard();
    void selectProject(QAction *);
    void reDrawAll();

    void updateDrawStyle();

private:
    /*!
     * \brief A population moved, so add it to the undostack
     *
     * This is called from the "itemWasMoved" slot when the item in
     * question is a population. This doesn't need to move the
     * population (that already happened), but it does need to record
     * this event in the undo stack.
     *
     * \param pops A vector of the populations which are on the move.
     */
    void populationMoved(const QVector <QSharedPointer<population> >& pops);

    /*!
     * Add an entry to the undo stack for the move of a projection
     * handle. The new location of the handle is obtained by querying
     * it - the projection is in selList at this point. The old
     * location of the handle is as near as makes no difference
     * lastLeftMouseDownPos.
     */
    void projectionHandleMoved();

    /*!
     * \brief Obtain the currently selected populations
     *
     * \return A vector of pointers to all currently selected
     * populations.
     */
    QVector <QSharedPointer<population> > currSelPopulations();

    /*!
     * \brief Obtain the currently selected population.
     *
     * Obtain the currently selected population if only a single
     * population is selected. If no populations are selected, return
     * NULL. If more than one population is selected, return NULL.
     *
     * \return pointer to the selected population, if only a single
     * population is selected; otherwise NULL.
     */
    QSharedPointer<population> currSelPopulation();

    /*!
     * Action to take when a mouse down event has changed selected objects.
     */
    void onNewSelections (float xGL, float yGL);

    /*!
     * \brief selListContains finds out if this->selList contains any of the members of objectList.
     * \param objectList The vector of systemObject pointers to look for in selList
     * \return true if selList contains anything from objectList, false otherwise.
     */
    bool selListContains (const QVector <QSharedPointer<systemObject> >& objectList);

    /*!
     * \brief delete any entries from this->selList which exist in
     * objectList.
     *
     * \param objectList the list of systemObject pointers to be deleted from sel
     */
    void deleteFromSelList (const QVector <QSharedPointer<systemObject> >& objectList);

    QString getUniquePopName(QString newName);
    // NB: This is unused. Refactor out.
    bool selChange;
    QPointF dragListStart;
    QRectF dragSelection;
    QDomDocument doc;
    QDomDocument tempDoc;

    /*!
     * This helps determine the position of an item BEFORE it was moved. it's
     * used for "undo move item position" - you need to know where the
     * item used to be so that you can restore it back to that
     * position. These are the "xGL,yGL" coordinates.
     *
     * Note that this is the last position of the *mouse*. It's used to get
     * the last position of the object by working out the
     * mouse offset (which can be obtained by calculating the mouse offset
     * at the end of the object movement).
     */
    QPointF lastLeftMouseDownPos;
};

#endif // ROOTDATA_H
