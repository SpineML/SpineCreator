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

#ifndef CINTERPRETER_H
#define CINTERPRETER_H

#include "globalHeader.h"

#include <vector>
using namespace std;

enum operationSet {
    VAL,
    OP,
    COND,
    BOOL_OP,
    FUNC,
    LBRACKET,
    RBRACKET,
    COMMA
};

enum opType {
    ADD,
    SUB,
    MULT,
    DIV
};

enum condType {
    EQ,
    GT,
    LT,
    GTEQ,
    LTEQ,
    NEQ,
    ERR
};




class lookup {

public:
    lookup(QString inName, float inVal) {value = inVal; name = inName;}
    QString name;
    float value;
};

struct valop {
    float val;
    operationSet op;
    float * ptr;
    bool isUnary;
};


bool isOperation(QString in);

opType getOpVal(QString in);

condType getCondVal(QString in);

QString doError(int err);

float getVarVal(QString in, vector <lookup> varList);

float * getVarPtr(QString in, vector <lookup> &varList);

float doFunction(float val1, float val2, float opf);

bool isVar(QString in);

bool isToken(QString in);

bool isNum(QString in);

bool isCondition(QString in);

bool isFunction(QString in);

float getFuncVal(QString in);

void printStack(vector <valop> opstackIn);
/*
float doCond(float val1, float val2, float type);

float doMaths(int startInd, int endInd, vector <valop> opstackIn);

QString doBrackets(int startInd, int endInd, vector <valop> opstackIn, float * outVal);

QString doConditional(int startInd, int endInd, vector <valop> opstackIn, float * outVal);

QString doBoolean(int startInd, int endInd, vector <valop> opstackIn, float * outVal);

QString doBoolBrackets(int startInd, int endInd, vector <valop> opstackIn, float * outVal);
*/
float interpretMaths(vector <valop>);

QString createStack(QString equation, vector <lookup> &varList, vector <valop> * returnStack);

#endif // CINTERPRETER_H
