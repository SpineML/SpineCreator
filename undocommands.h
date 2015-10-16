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

#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <utility>
#include "globalHeader.h"
#include "projections.h"
#include "genericinput.h"
#include "connection.h"
#include "experiment.h"

class delSelection : public QUndoCommand
{
public:
    delSelection(rootData * data, QVector <QSharedPointer<systemObject> > list, QUndoCommand *parent = 0);
    ~delSelection() {/*qDebug() << "Cleaning up selection";*/}
    void undo();
    void redo();

private:
    rootData * data;
};

class addPopulationCmd : public QUndoCommand
{
public:
    addPopulationCmd(rootData * data, QSharedPointer <population> pop, QUndoCommand *parent = 0);
    ~addPopulationCmd() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <population> pop;
    int selIndex;
    bool isDeleted;
};

class delPopulation : public QUndoCommand
{
public:
    delPopulation(rootData * data, QSharedPointer <population> pop, QUndoCommand *parent = 0);
    ~delPopulation() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <population> pop;
    int index;
    int selIndex;
    bool isDeleted;
};

class moveProjectionHandle : public QUndoCommand
{
public:
    moveProjectionHandle(rootData * data, QSharedPointer <projection> proj,
                         const QPointF& oldPos, const QPointF& newPos,
                         QUndoCommand *parent = 0);
    ~moveProjectionHandle() {};
    void undo();
    void redo();
private:
    /*!
     * The rootData object, which is included so that the screen can
     * be re-drawn after restoring the position of the handle with
     * the undo/redo methods.
     */
    rootData * data;

    /*!
     * Pointer to the projection whose handle has been moved
     */
    QSharedPointer <projection> proj;

    /*!
     * The old position of the handle, before the move.
     */
    QPointF oldPos;

    /*!
     * The new position of the handle, after the move.
     */
    QPointF newPos;
};

class movePopulation : public QUndoCommand
{
public:
    movePopulation(rootData * data, QSharedPointer <population> pop,
                   const QPointF& oldPos, const QPointF& newPos,
                   QUndoCommand *parent = 0);
    ~movePopulation() {}
    void undo();
    void redo();

private:

    /*!
     * The rootData object, which is included so that the screen can
     * be re-drawn after restoring the position of the population with
     * the undo/redo methods.
     */
    rootData * data;

    /*!
     * Pointer to the population which has been moved
     */
    QSharedPointer <population> pop;

    /*!
     * The old position of the population, before the move.
     */
    QPointF oldPos;

    /*!
     * The new position of the population, after the move.
     */
    QPointF newPos;
};

class addProjection : public QUndoCommand
{
public:
    addProjection(rootData * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~addProjection() {if (isDeleted) proj->remove(data);}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <projection> proj;
    int selIndex;
    bool isDeleted;
};

class delProjection : public QUndoCommand
{
public:
    delProjection(rootData * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~delProjection() {if (isDeleted) {/*qDebug() << "Cleaning up projection";*/ proj->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <projection> proj;
    bool isChild;
    int selIndex;
    bool isDeleted;
};

class addSynapse : public QUndoCommand
{
public:
    addSynapse(rootData * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~addSynapse() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <projection> proj;
    QSharedPointer <synapse> syn;
    bool isDeleted;
};

class delSynapse : public QUndoCommand
{
public:
    delSynapse(rootData * data, QSharedPointer <projection> proj, QSharedPointer <synapse> synapse, QUndoCommand *parent = 0);
    ~delSynapse() {/*if (!isUndone) {qDebug() << "Cleaning up synapse"; delete syn;}*/}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <projection> proj;
    QSharedPointer <synapse> syn;
    int projPos;
    bool isUndone;
    bool isChild;
};

class addInput : public QUndoCommand
{
public:
    addInput(rootData * data, QSharedPointer <NineMLComponentData> src, QSharedPointer <NineMLComponentData> dst, QUndoCommand *parent = 0);
    ~addInput() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer<genericInput> input;
    QSharedPointer <NineMLComponentData> src;
    QSharedPointer <NineMLComponentData> dst;
    bool isDeleted;
};

class delInput : public QUndoCommand
{
public:
    delInput(rootData * data, QSharedPointer<genericInput> input, QUndoCommand *parent = 0);
    ~delInput() {if (isDeleted) {/*qDebug() << "Cleaning up input " << input->projInput;*/ input->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer<genericInput> input;
    int selIndex;
    bool isChild;
    bool isDeleted;
};

class changeConnection : public QUndoCommand
{
public:
    changeConnection(rootData * data, QSharedPointer<systemObject> ptr, int index, QUndoCommand *parent = 0);
    ~changeConnection() {if (!isUndone) delete oldConn;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer<systemObject> ptr;
    int index;
    QString scriptName;
    connection * oldConn;
    bool isUndone;
};

class setSizeUndo : public QUndoCommand
{
public:
    setSizeUndo(rootData * data, QSharedPointer <population> ptr, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <population> ptr;
    int oldValue;
    int value;
    bool firstRedo;
};

class setLoc3Undo : public QUndoCommand
{
public:
    setLoc3Undo(rootData * data, QSharedPointer <population> ptr, int index, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <population> ptr;
    int oldValue;
    int value;
    int index;
};

class updateParUndo : public QUndoCommand
{
public:
    updateParUndo(rootData * data, ParameterData * ptr, int index, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    ParameterData * ptr;
    float oldValue;
    float value;
    int index;
    bool firstRedo;
};

class updateConnProb: public QUndoCommand
{
public:
    updateConnProb(rootData * data, fixedProb_connection * ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    fixedProb_connection * ptr;
    float oldValue;
    float value;
    bool firstRedo;
};

class undoUpdatePythonConnectionScriptPar: public QUndoCommand
{
public:
    undoUpdatePythonConnectionScriptPar(rootData * data, pythonscript_connection * ptr, float new_val, QString par_name, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    pythonscript_connection * ptr;
    float oldValue;
    float value;
    QString par_name;
    bool firstRedo;
};

class undoUpdatePythonConnectionScriptProp: public QUndoCommand
{
public:
    undoUpdatePythonConnectionScriptProp(rootData * data, pythonscript_connection * ptr, QString par_name, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    pythonscript_connection * ptr;
    QString oldProp;
    QString par_name;
    bool firstRedo;
};

class updateParType : public QUndoCommand
{
public:
    updateParType(rootData * data, ParameterData * ptr, QString newType, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    ParameterData * ptr;
    ParameterType oldType;
    ParameterType newType;
    QVector <double> oldValues;
};

class updateTitle : public QUndoCommand
{
public:
    updateTitle(QSharedPointer <population> ptr, QString newName, QString oldName, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    QSharedPointer <population> ptr;
    QString oldName;
    QString newName;
};

class updateProjDrawStyle : public QUndoCommand
{
public:
    updateProjDrawStyle(QSharedPointer <projection> ptr, drawStyle newStyle, drawStyle oldStyle, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    QSharedPointer <projection> ptr;
    drawStyle oldStyle;
    drawStyle newStyle;
};

class updateProjShowLabel : public QUndoCommand
{
public:
    updateProjShowLabel(QSharedPointer <projection> ptr, bool newShowLabel, bool oldShowLabel, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    QSharedPointer <projection> ptr;
    bool oldShowLabel;
    bool newShowLabel;
};

class updateModelTitle : public QUndoCommand
{
public:
    updateModelTitle(rootData * data, QString newName, projectObject * project, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QString oldName;
    QString newName;
    projectObject * project;
};

class updateComponentTypeUndo : public QUndoCommand
{
public:
    updateComponentTypeUndo(rootData * data, QSharedPointer <NineMLComponentData> componentData, QSharedPointer<NineMLComponent> newComponent, QUndoCommand *parent = 0);
    ~updateComponentTypeUndo();
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QVector < ParameterData * > oldParDatas;
    QVector < StateVariableData * > oldSVDatas;
    QVector < ParameterData * > newParDatas;
    QVector < StateVariableData * > newSVDatas;
    QSharedPointer<NineMLComponent> oldComponent;
    QSharedPointer<NineMLComponent> newComponent;
    QSharedPointer <NineMLComponentData> componentData;
    QVector <QString> srcPortsInputs;
    QVector <QString> dstPortsInputs;
    QVector <QString> srcPortsOutputs;
    QVector <QString> dstPortsOutputs;
    bool isRedone;
};

class updateLayoutMinDist: public QUndoCommand
{
public:
    updateLayoutMinDist(rootData * data, QSharedPointer<NineMLLayoutData> ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer<NineMLLayoutData> ptr;
    float oldValue;
    float value;
};

class updateLayoutSeed: public QUndoCommand
{
public:
    updateLayoutSeed(rootData * data, QSharedPointer<NineMLLayoutData> ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer<NineMLLayoutData> ptr;
    float oldValue;
    float value;
};

class pastePars: public QUndoCommand
{
public:
    pastePars(rootData * data, QSharedPointer <NineMLComponentData> source, QSharedPointer <NineMLComponentData> dest, QUndoCommand *parent = 0);
    ~pastePars() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    QSharedPointer <NineMLComponentData> oldData;
    QSharedPointer <NineMLComponentData> source;
    QSharedPointer <NineMLComponentData> dest;
};

///// components

struct viewCLstruct;

class changeComponent: public QUndoCommand
{
public:
    changeComponent(RootComponentItem *root, QSharedPointer<NineMLComponent> oldComponent, QString message, QUndoCommand *parent = 0);
    ~changeComponent() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    QSharedPointer<NineMLComponent> changedComponent;
    QSharedPointer<NineMLComponent> unChangedComponent;
    bool first_redo;
};

class changeComponentType: public QUndoCommand
{
public:
    changeComponentType(RootComponentItem *root, QVector <QSharedPointer<NineMLComponent> > * old_lib, QVector <QSharedPointer<NineMLComponent> > * new_lib, QSharedPointer<NineMLComponent> component, QString message, QUndoCommand *parent = 0);
    ~changeComponentType() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    QVector <QSharedPointer<NineMLComponent> > * old_lib;
    QVector <QSharedPointer<NineMLComponent> > * new_lib;
    QSharedPointer<NineMLComponent> component;
    bool first_redo;
};

class deleteOutputUndo: public QUndoCommand
{
public:
    deleteOutputUndo(rootData *data, experiment * expt, exptOutput * output, QUndoCommand *parent = 0);
    ~deleteOutputUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    experiment * expt;
    exptOutput * output;
    int location;
};

class deleteInputUndo: public QUndoCommand
{
public:
    deleteInputUndo(rootData *data, experiment * expt, exptInput * input, QUndoCommand *parent = 0);
    ~deleteInputUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    experiment * expt;
    exptInput * input;
    int location;
};

class deleteChangePropUndo: public QUndoCommand
{
public:
    deleteChangePropUndo(rootData *data, experiment * expt, exptChangeProp * prop, QUndoCommand *parent = 0);
    ~deleteChangePropUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    experiment * expt;
    exptChangeProp * prop;
    int location;
};

class deleteLesionUndo: public QUndoCommand
{
public:
    deleteLesionUndo(rootData *data, experiment * expt, exptLesion * lesion, QUndoCommand *parent = 0);
    ~deleteLesionUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    experiment * expt;
    exptLesion * lesion;
    int location;
};

#endif // UNDOCOMMANDS_H
