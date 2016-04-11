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
#include "NL_projection_and_synapse.h"
#include "NL_genericinput.h"
#include "NL_connection.h"
#include "EL_experiment.h"

class delSelection : public QUndoCommand
{
public:
    delSelection(nl_rootdata * data, QVector <QSharedPointer<systemObject> > list, QUndoCommand *parent = 0);
    ~delSelection() {/*qDebug() << "Cleaning up selection";*/}
    void undo();
    void redo();

private:
    nl_rootdata * data;
};

class addPopulationCmd : public QUndoCommand
{
public:
    addPopulationCmd(nl_rootdata * data, QSharedPointer <population> pop, QUndoCommand *parent = 0);
    ~addPopulationCmd() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <population> pop;
    int selIndex;
    bool isDeleted;
};

class delPopulation : public QUndoCommand
{
public:
    delPopulation(nl_rootdata * data, QSharedPointer <population> pop, QUndoCommand *parent = 0);
    ~delPopulation() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <population> pop;
    int index;
    int selIndex;
    bool isDeleted;
};

class moveProjectionHandle : public QUndoCommand
{
public:
    moveProjectionHandle(nl_rootdata * data, QSharedPointer <projection> proj,
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
    nl_rootdata * data;

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
    movePopulation(nl_rootdata * data, QSharedPointer <population> pop,
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
    nl_rootdata * data;

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
    addProjection(nl_rootdata * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~addProjection() {if (isDeleted) proj->remove(data);}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <projection> proj;
    int selIndex;
    bool isDeleted;
};

class delProjection : public QUndoCommand
{
public:
    delProjection(nl_rootdata * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~delProjection() {if (isDeleted) {/*qDebug() << "Cleaning up projection";*/ proj->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <projection> proj;
    bool isChild;
    int selIndex;
    bool isDeleted;
};

class addSynapse : public QUndoCommand
{
public:
    addSynapse(nl_rootdata * data, QSharedPointer <projection> proj, QUndoCommand *parent = 0);
    ~addSynapse() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <projection> proj;
    QSharedPointer <synapse> syn;
    bool isDeleted;
};

class delSynapse : public QUndoCommand
{
public:
    delSynapse(nl_rootdata * data, QSharedPointer <projection> proj, QSharedPointer <synapse> synapse, QUndoCommand *parent = 0);
    ~delSynapse() {/*if (!isUndone) {qDebug() << "Cleaning up synapse"; delete syn;}*/}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <projection> proj;
    QSharedPointer <synapse> syn;
    int projPos;
    bool isUndone;
    bool isChild;
};

class addInput : public QUndoCommand
{
public:
    addInput(nl_rootdata * data, QSharedPointer <ComponentInstance> src, QSharedPointer <ComponentInstance> dst, QUndoCommand *parent = 0);
    ~addInput() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer<genericInput> input;
    QSharedPointer <ComponentInstance> src;
    QSharedPointer <ComponentInstance> dst;
    bool isDeleted;
};

class delInput : public QUndoCommand
{
public:
    delInput(nl_rootdata * data, QSharedPointer<genericInput> input, QUndoCommand *parent = 0);
    ~delInput() {if (isDeleted) {/*qDebug() << "Cleaning up input " << input->projInput;*/ input->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer<genericInput> input;
    int selIndex;
    bool isChild;
    bool isDeleted;
};

class changeConnection : public QUndoCommand
{
public:
    changeConnection(nl_rootdata * data, QSharedPointer<systemObject> ptr, int index, QUndoCommand *parent = 0);
    ~changeConnection() {if (!isUndone) delete oldConn;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer<systemObject> ptr;
    int index;
    QString scriptName;
    connection * oldConn;
    bool isUndone;
};

class setSizeUndo : public QUndoCommand
{
public:
    setSizeUndo(nl_rootdata * data, QSharedPointer <population> ptr, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <population> ptr;
    int oldValue;
    int value;
    bool firstRedo;
};

class setLoc3Undo : public QUndoCommand
{
public:
    setLoc3Undo(nl_rootdata * data, QSharedPointer <population> ptr, int index, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <population> ptr;
    int oldValue;
    int value;
    int index;
};

class updateParUndo : public QUndoCommand
{
public:
    updateParUndo(nl_rootdata * data, ParameterInstance * ptr, int index, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    ParameterInstance * ptr;
    float oldValue;
    float value;
    int index;
    bool firstRedo;
};

class updateConnProb: public QUndoCommand
{
public:
    updateConnProb(nl_rootdata * data, fixedProb_connection * ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    fixedProb_connection * ptr;
    float oldValue;
    float value;
    bool firstRedo;
};

class undoUpdatePythonConnectionScriptPar: public QUndoCommand
{
public:
    undoUpdatePythonConnectionScriptPar(nl_rootdata * data, pythonscript_connection * ptr, float new_val, QString par_name, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    pythonscript_connection * ptr;
    float oldValue;
    float value;
    QString par_name;
    bool firstRedo;
};

class undoUpdatePythonConnectionScriptProp: public QUndoCommand
{
public:
    undoUpdatePythonConnectionScriptProp(nl_rootdata * data, pythonscript_connection * ptr, QString par_name, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    pythonscript_connection * ptr;
    QString oldProp;
    QString par_name;
    bool firstRedo;
};

class updateParType : public QUndoCommand
{
public:
    updateParType(nl_rootdata * data, ParameterInstance * ptr, QString newType, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    ParameterInstance * ptr;
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
    updateModelTitle(nl_rootdata * data, QString newName, projectObject * project, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QString oldName;
    QString newName;
    projectObject * project;
};

class updateComponentTypeUndo : public QUndoCommand
{
public:
    updateComponentTypeUndo(nl_rootdata * data, QSharedPointer <ComponentInstance> componentData, QSharedPointer<Component> newComponent, QUndoCommand *parent = 0);
    ~updateComponentTypeUndo();
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QVector < ParameterInstance * > oldParDatas;
    QVector < StateVariableInstance * > oldSVDatas;
    QVector < ParameterInstance * > newParDatas;
    QVector < StateVariableInstance * > newSVDatas;
    QSharedPointer<Component> oldComponent;
    QSharedPointer<Component> newComponent;
    QSharedPointer <ComponentInstance> componentData;
    QVector <QString> srcPortsInputs;
    QVector <QString> dstPortsInputs;
    QVector <QString> srcPortsOutputs;
    QVector <QString> dstPortsOutputs;
    bool isRedone;
};

class updateLayoutMinDist: public QUndoCommand
{
public:
    updateLayoutMinDist(nl_rootdata * data, QSharedPointer<NineMLLayoutData> ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer<NineMLLayoutData> ptr;
    float oldValue;
    float value;
};

class updateLayoutSeed: public QUndoCommand
{
public:
    updateLayoutSeed(nl_rootdata * data, QSharedPointer<NineMLLayoutData> ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer<NineMLLayoutData> ptr;
    float oldValue;
    float value;
};

class pastePars: public QUndoCommand
{
public:
    pastePars(nl_rootdata * data, QSharedPointer <ComponentInstance> source, QSharedPointer <ComponentInstance> dest, QUndoCommand *parent = 0);
    ~pastePars() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    QSharedPointer <ComponentInstance> oldData;
    QSharedPointer <ComponentInstance> source;
    QSharedPointer <ComponentInstance> dest;
};

///// components

struct viewCLstruct;

class changeComponent: public QUndoCommand
{
public:
    changeComponent(RootComponentItem *root, QSharedPointer<Component> oldComponent, QString message, QUndoCommand *parent = 0);
    ~changeComponent() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    QSharedPointer<Component> changedComponent;
    QSharedPointer<Component> unChangedComponent;
    bool first_redo;
};

class changeComponentType: public QUndoCommand
{
public:
    changeComponentType(RootComponentItem *root, QVector <QSharedPointer<Component> > * old_lib, QVector <QSharedPointer<Component> > * new_lib, QSharedPointer<Component> component, QString message, QUndoCommand *parent = 0);
    ~changeComponentType() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    QVector <QSharedPointer<Component> > * old_lib;
    QVector <QSharedPointer<Component> > * new_lib;
    QSharedPointer<Component> component;
    bool first_redo;
};

class deleteOutputUndo: public QUndoCommand
{
public:
    deleteOutputUndo(nl_rootdata *data, experiment * expt, exptOutput * output, QUndoCommand *parent = 0);
    ~deleteOutputUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    experiment * expt;
    exptOutput * output;
    int location;
};

class deleteInputUndo: public QUndoCommand
{
public:
    deleteInputUndo(nl_rootdata *data, experiment * expt, exptInput * input, QUndoCommand *parent = 0);
    ~deleteInputUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    experiment * expt;
    exptInput * input;
    int location;
};

class deleteChangePropUndo: public QUndoCommand
{
public:
    deleteChangePropUndo(nl_rootdata *data, experiment * expt, exptChangeProp * prop, QUndoCommand *parent = 0);
    ~deleteChangePropUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    experiment * expt;
    exptChangeProp * prop;
    int location;
};

class deleteLesionUndo: public QUndoCommand
{
public:
    deleteLesionUndo(nl_rootdata *data, experiment * expt, exptLesion * lesion, QUndoCommand *parent = 0);
    ~deleteLesionUndo() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    nl_rootdata * data;
    experiment * expt;
    exptLesion * lesion;
    int location;
};

#endif // UNDOCOMMANDS_H
