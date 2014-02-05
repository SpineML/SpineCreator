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
**           Author: Alex Cope                                            **
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

class delSelection : public QUndoCommand
{
public:
    delSelection(rootData * data, vector <systemObject * > list, QUndoCommand *parent = 0);
    ~delSelection() {/*qDebug() << "Cleaning up selection";*/}
    void undo();
    void redo();

private:
    rootData * data;
};

class addPopulationCmd : public QUndoCommand
{
public:
    addPopulationCmd(rootData * data, population * pop, QUndoCommand *parent = 0);
    ~addPopulationCmd() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    population * pop;
    int selIndex;
    bool isDeleted;
};

class delPopulation : public QUndoCommand
{
public:
    delPopulation(rootData * data, population * pop, QUndoCommand *parent = 0);
    ~delPopulation() {if (isDeleted) {/*qDebug() << "Cleaning up population: " << pop->getName();*/ pop->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    population * pop;
    int index;
    int selIndex;
    bool isDeleted;
};

class moveProjectionHandle : public QUndoCommand
{
public:
    moveProjectionHandle(rootData * data, projection * proj,
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
    projection * proj;

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
    movePopulation(rootData * data, population * pop,
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
    population * pop;

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
    addProjection(rootData * data, projection * proj, QUndoCommand *parent = 0);
    ~addProjection() {if (isDeleted) proj->remove(data);}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    projection * proj;
    int selIndex;
    bool isDeleted;
};

class delProjection : public QUndoCommand
{
public:
    delProjection(rootData * data, projection * proj, QUndoCommand *parent = 0);
    ~delProjection() {if (isDeleted) {/*qDebug() << "Cleaning up projection";*/ proj->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    projection * proj;
    bool isChild;
    int selIndex;
    bool isDeleted;
};

class addSynapse : public QUndoCommand
{
public:
    addSynapse(rootData * data, projection * proj, QUndoCommand *parent = 0);
    ~addSynapse() {if (isDeleted) delete syn;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    projection * proj;
    synapse * syn;
    bool isDeleted;
};

class delSynapse : public QUndoCommand
{
public:
    delSynapse(rootData * data, projection * proj, synapse * synapse, QUndoCommand *parent = 0);
    ~delSynapse() {/*if (!isUndone) {qDebug() << "Cleaning up synapse"; delete syn;}*/}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    projection * proj;
    synapse * syn;
    int projPos;
    bool isUndone;
    bool isChild;
};

class addInput : public QUndoCommand
{
public:
    addInput(rootData * data, NineMLComponentData * src, NineMLComponentData * dst, QUndoCommand *parent = 0);
    ~addInput() {if (isDeleted) delete input;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    genericInput * input;
    NineMLComponentData * src;
    NineMLComponentData * dst;
    bool isDeleted;
};

class delInput : public QUndoCommand
{
public:
    delInput(rootData * data, genericInput * input, QUndoCommand *parent = 0);
    ~delInput() {if (isDeleted) {/*qDebug() << "Cleaning up input " << input->projInput;*/ input->remove(data);}}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    genericInput * input;
    int selIndex;
    bool isChild;
    bool isDeleted;
};

class changeConnection : public QUndoCommand
{
public:
    changeConnection(rootData * data, systemObject * ptr, int index, QUndoCommand *parent = 0);
    ~changeConnection() {if (!isUndone) delete oldConn;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    systemObject * ptr;
    int index;
    connection * oldConn;
    bool isUndone;
};

class setSizeUndo : public QUndoCommand
{
public:
    setSizeUndo(rootData * data, population * ptr, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    population * ptr;
    int oldValue;
    int value;
};

class setLoc3Undo : public QUndoCommand
{
public:
    setLoc3Undo(rootData * data, population * ptr, int index, int value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    population * ptr;
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

class updateConnEquation: public QUndoCommand
{
public:
    updateConnEquation(rootData * data, distanceBased_connection * ptr, QString newEq, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    distanceBased_connection * ptr;
    QString oldValue;
    QString value;
    bool firstRedo;
};

class updateConnDelayEquation: public QUndoCommand
{
public:
    updateConnDelayEquation(rootData * data, distanceBased_connection * ptr, QString newEq, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    distanceBased_connection * ptr;
    QString oldValue;
    QString value;
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
    vector<float> oldValues;
};

class updateTitle : public QUndoCommand
{
public:
    updateTitle(population * ptr, QString newName, QString oldName, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    population * ptr;
    QString oldName;
    QString newName;
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

class updateComponentType : public QUndoCommand
{
public:
    updateComponentType(rootData * data, NineMLComponentData * componentData, NineMLComponent * newComponent, QUndoCommand *parent = 0);
    ~updateComponentType();
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    vector < ParameterData * > oldParDatas;
    vector < StateVariableData * > oldSVDatas;
    vector < ParameterData * > newParDatas;
    vector < StateVariableData * > newSVDatas;
    NineMLComponent * oldComponent;
    NineMLComponent * newComponent;
    NineMLComponentData * componentData;
    vector <QString> srcPortsInputs;
    vector <QString> dstPortsInputs;
    vector <QString> srcPortsOutputs;
    vector <QString> dstPortsOutputs;
    bool isRedone;
};

class updateLayoutMinDist: public QUndoCommand
{
public:
    updateLayoutMinDist(rootData * data, NineMLLayoutData * ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    NineMLLayoutData * ptr;
    float oldValue;
    float value;
};

class updateLayoutSeed: public QUndoCommand
{
public:
    updateLayoutSeed(rootData * data, NineMLLayoutData * ptr, float value, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    NineMLLayoutData * ptr;
    float oldValue;
    float value;
};

class pastePars: public QUndoCommand
{
public:
    pastePars(rootData * data, NineMLComponentData * source, NineMLComponentData * dest, QUndoCommand *parent = 0);
    ~pastePars() {delete oldData; delete source;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    rootData * data;
    NineMLComponentData * oldData;
    NineMLComponentData * source;
    NineMLComponentData * dest;
};

///// components

struct viewCLstruct;

class changeComponent: public QUndoCommand
{
public:
    changeComponent(RootComponentItem *root, NineMLComponent * oldComponent, QString message, QUndoCommand *parent = 0);
    ~changeComponent() {delete changedComponent; delete unChangedComponent;}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    NineMLComponent * changedComponent;
    NineMLComponent * unChangedComponent;
    bool first_redo;
};

class changeComponentType: public QUndoCommand
{
public:
    changeComponentType(RootComponentItem *root, vector <NineMLComponent *> * old_lib, vector <NineMLComponent *> * new_lib, NineMLComponent * component, QString message, QUndoCommand *parent = 0);
    ~changeComponentType() {}
    void undo();
    void redo();

private:
    // these references are needed for the redo and undo
    viewCLstruct * viewCL;
    vector <NineMLComponent *> * old_lib;
    vector <NineMLComponent *> * new_lib;
    NineMLComponent * component;
    bool first_redo;
};

#endif // UNDOCOMMANDS_H
