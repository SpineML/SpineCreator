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

#include "cinterpreter.h"
//#include "G__ci.h"

//QString QString::number(float);
#include "nineML_classes.h"
#include "globalHeader.h"
//#include "stringify.h"

int precedance(valop in) {

    if (in.val == ADD || in.val == SUB) return 0;
    if (in.val == MULT || in.val == DIV) return 1;

    // oops
    return -1;
}

bool isOperation(QString in) {

    QString operations = "+-*/";

    if (operations.contains(in[0])) {
        return true;
    }
    return false;

}

opType getOpVal(QString in) {

    if (in[0] == '+') return ADD;
    if (in[0] == '-') return SUB;
    if (in[0] == '*') return MULT;
    if (in[0] == '/') return DIV;

    return ADD;

}

condType getCondVal(QString in) {

    if (in == "==") return EQ;
    if (in == ">") return GT;
    if (in == "<") return LT;
    if (in == ">=") return GTEQ;
    if (in == "<=") return LTEQ;
    if (in == "!=") return NEQ;

    return ERR;

}

bool isVar(QString in) {

    QString letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

    if (letters.contains(in[0])) {
        return true;
    }
    return false;

}

bool isToken(QString in) {

    // once we are in a var we have a wider range of possible chars
    QString tokenItems = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890#";

    if (tokenItems.contains(in[0])) {
        return true;
    }
    return false;

}

bool isNum(QString in) {

    QString nums = "0123456789.";

    if (nums.contains(in[0])) {
        return true;
    }
    return false;

}

bool isCondition(QString in) {

    QString nums = "=><!";

    if (nums.contains(in[0])) {
        return true;
    }
    return false;

}

bool isFunction(QString in) {

    QStringList functions;
    functions.push_back("pow");
    functions.push_back("exp");
    functions.push_back("sin");
    functions.push_back("cos");
    functions.push_back("log");
    functions.push_back("log10");
    functions.push_back("sinh");
    functions.push_back("cosh");
    functions.push_back("tanh");
    functions.push_back("sqrt");
    functions.push_back("atan");
    functions.push_back("asin");
    functions.push_back("acos");
    functions.push_back("asinh");
    functions.push_back("acosh");
    functions.push_back("atanh");
    functions.push_back("atan2");
    functions.push_back("ceil");
    functions.push_back("floor");
    functions.push_back("rand");
    functions.push_back("mod");

    for (unsigned int i = 0; i < (uint) functions.size(); ++i) {
        if (in == functions[i]) {
            return true;
        }
    }
    return false;

}

float getFuncVal(QString in) {

    QStringList functions;
    functions.push_back("pow");
    functions.push_back("exp");
    functions.push_back("sin");
    functions.push_back("cos");
    functions.push_back("log");
    functions.push_back("log10");
    functions.push_back("sinh");
    functions.push_back("cosh");
    functions.push_back("tanh");
    functions.push_back("sqrt");
    functions.push_back("atan");
    functions.push_back("asin");
    functions.push_back("acos");
    functions.push_back("asinh");
    functions.push_back("acosh");
    functions.push_back("atanh");
    functions.push_back("atan2");
    functions.push_back("ceil");
    functions.push_back("floor");
    functions.push_back("rand");
    functions.push_back("mod");


    for (unsigned int i = 0; i < (uint) functions.size(); ++i) {
        if (in == functions[i]) {
            return float(i);
        }
    }
    return -1;

}

bool isFuncUnary(QString in) {

    if (in == "pow" || in == "atan2" || in == "mod" )
        return false;

    return true;

}


QString doError(int err) {

    if (err == 1) {
        return "Mismatched brackets";
    }
    if (err == 2) {
        return "Found undefined variable";
    }
    if (err == 3) {
        return "Unrecognised conditional";
    }
    if (err == 4) {
        return "Truncated - boolean left hanging";
    }
    if (err == 5) {
        return "Bad boolean operator - use && or ||";
    }
    if (err == 6) {
        return "Incorrect maths";
    }
    if (err == 7) {
        return "Divide by zero";
    }
    if (err == 8) {
        return "Incorrect use of a function";
    }
    if (err == 9) {
        return "Boolean found without a conditional";
    }
    if (err == 10) {
        return "Too many conditionals in a section";
    }
    if (err == 11) {
        return "Failed to evaluate";
    }
    if (err == 12) {
        return "Something is pretty wrong here - we have failed to return from a function correctly";
    }
    if (err == 13) {
        return "Unrecognised function";
    }


    return "Unrecognised error code";
}

float getVarVal(QString in, vector <lookup> varList) {

    vector < lookup > builtin;
    builtin.push_back(lookup("e", M_E));
    builtin.push_back(lookup("pi", M_PI));

    for (unsigned int i = 0; i < builtin.size(); ++i) {
        if (in == builtin[i].name) {
            return builtin[i].value;
        }
    }

    for (unsigned int i = 0; i < varList.size(); ++i) {
        if (in == varList[i].name) {
            return varList[i].value;
        }
    }

    return INFINITY;

}

float * getVarPtr(QString in, vector <lookup> &varList) {

    /*vector < lookup > builtin;
    builtin.push_back(lookup("e", M_E));
    builtin.push_back(lookup("pi", M_PI));

    for (unsigned int i = 0; i < builtin.size(); ++i) {
        if (in == builtin[i].name) {
            return builtin[i].value;
        }
    }*/

    for (unsigned int i = 0; i < varList.size(); ++i) {
        if (in == varList[i].name) {
            return &(varList[i].value);
        }
    }

    return NULL;

}

float doFunction(float val1, float val2, float opf) {

    int op = int(opf);

    if (op == 0 && val2 == INFINITY) return INFINITY;
    if (op == 16 && val2 == INFINITY) return INFINITY;
    if (op == 20 && val2 == INFINITY) return INFINITY;
    if (op != 0 && op != 16 && op != 20 && val2 != INFINITY) return INFINITY;

    if (op == 0) return pow(val1, val2);
    if (op == 1) return exp(val1);
    if (op == 2) return sin(val1);
    if (op == 3) return cos(val1);
    if (op == 4) return log(val1);
    if (op == 5) return log10(val1);
    if (op == 6) return sinh(val1);
    if (op == 7) return cosh(val1);
    if (op == 8) return tanh(val1);
    if (op == 9) return sqrt(val1);
    if (op == 10) return atan(val1);
    if (op == 11) return asin(val1);
    if (op == 12) return acos(val1);
    if (op == 13) return asinh(val1);
    if (op == 14) return acosh(val1);
    if (op == 15) return atanh(val1);
    if (op == 16) return atan2(val1, val2);
    if (op == 17) return ceil(val1);
    if (op == 18) return floor(val1);
    if (op == 19) return float(rand())/RAND_MAX;
    if (op == 20) return fmod(val1, val2);

    return 0;

}

void printStack(vector <valop> opstackIn) {

    cerr << "STACK: \n";
    for (unsigned int i = 0; i < opstackIn.size(); ++i) {
        cerr << float(i) << ": ";
        cerr << float(opstackIn[i].op) << " " << float(opstackIn[i].val) << "\n";
    }

}
/*
float doCond(float val1, float val2, float type) {

    if (type == EQ) {if (val1 == val2) {return 1;} else {return 0;}}
    if (type == GT) {if (val1 > val2) {return 1;} else {return 0;}}
    if (type == LT) {if (val1 < val2) {return 1;} else {return 0;}}
    if (type == GTEQ) {if (val1 >= val2) {return 1;} else {return 0;}}
    if (type == LTEQ) {if (val1 <= val2) {return 1;} else {return 0;}}
    if (type == NEQ) {if (val1 != val2) {return 1;} else {return 0;}}

    return 0;
}

float doMaths(int startInd, int endInd, vector <valop> opstackIn) {

    vector <valop> opstack;
    for (unsigned int i = (uint) startInd; i < (uint) endInd; ++i) {
        opstack.push_back(opstackIn[i]);
    }

    // multiplication and division first
    for (unsigned int i = 0; i < opstack.size(); ++i) {

        //printStack(opstack);

        valop newValOp;
        if (opstack[i].op == OP && opstack[i].val == float(MULT)) {
            // can't have a * as the first thing
            if (i == 0) return INFINITY;
            if (i + 1 == opstack.size()) return INFINITY;
            if (opstack[i-1].op == VAL && opstack[i+1].op == VAL) {
                newValOp.op = VAL;
                newValOp.val = opstack[i-1].val * opstack[i+1].val;
                opstack.erase(opstack.begin()+i-1, opstack.begin()+i+2);
                opstack.insert(opstack.begin()+i-1, newValOp);
                // shunt back i as we took out 2 behind but added 1 behind
                i -= 1;
            } else {
                // can't do an operation on non-values
                return INFINITY;
            }

        } else if (opstack[i].op == OP && opstack[i].val == float(DIV)) {
            // can't have a / as the first thing
            if (i == 0) return INFINITY;
            if (i + 1 == opstack.size()) return INFINITY;
            if (opstack[i-1].op == VAL && opstack[i+1].op == VAL) {
                // no divide by zero
                if (opstack[i+1].val == 0) return -INFINITY;
                newValOp.op = VAL;
                newValOp.val = opstack[i-1].val / opstack[i+1].val;
                opstack.erase(opstack.begin()+i-1, opstack.begin()+i+2);
                opstack.insert(opstack.begin()+i-1,newValOp);
                // shunt back i as we took out 2 behind but added 1 behind
                i -= 1;
            } else {
                // can't do an operation on non-values
                return INFINITY;
            }
        }
    }

    // now onto addition and subtraction:
    for (unsigned int i = 0; i < opstack.size(); ++i) {
        valop newValOp;
        if (opstack[i].op == OP && opstack[i].val == float(ADD)) {
            // if we have an add first it makes no odds, so just remove it
            if (i == 0) {
                opstack.erase(opstack.begin());
                continue;
            }
            if (i + 1 == opstack.size()) return INFINITY;
            if (opstack[i-1].op == VAL && opstack[i+1].op == VAL) {
                newValOp.op = VAL;
                newValOp.val = opstack[i-1].val + opstack[i+1].val;
                opstack.erase(opstack.begin()+i-1, opstack.begin()+i+2);
                opstack.insert(opstack.begin()+i-1, newValOp);
                // shunt back i as we took out 2 behind but added 1 behind
                i -= 1;
            } else {
                // can't do an operation on non-values
                return INFINITY;
            }

        } else if (opstack[i].op == OP && opstack[i].val == float(SUB)) {
            // if - is first thing then negative next val
            if (i == 0) {
                if (opstack[i+1].op == VAL) {
                    opstack[i+1].val *= -1;
                    opstack.erase(opstack.begin());
                    continue;
                }
                else return INFINITY;
            }
            if (i + 1 == opstack.size()) return INFINITY;
            if (opstack[i-1].op == VAL && opstack[i+1].op == VAL) {
                newValOp.op = VAL;
                newValOp.val = opstack[i-1].val - opstack[i+1].val;
                opstack.erase(opstack.begin()+i-1, opstack.begin()+i+2);
                opstack.insert(opstack.begin()+i-1,newValOp);
                // shunt back i as we took out 2 behind but added 1 behind
                i -= 1;
            } else {
                // can't do an operation on non-values
                return INFINITY;
            }
        }
    }

    // ok, we should now have an opstack of size 1, who only contains a value, which we send back!
    if (opstack.size() != 1) return INFINITY;
    if (opstack[0].op != float(VAL)) return INFINITY;

    // all good - send it back
    return opstack[0].val;
}

QString doBrackets(int startInd, int endInd, vector <valop> opstackIn, float * outVal)  {


    vector <valop> opstack;
    for (unsigned int i = (uint) startInd; i < (uint) endInd; ++i) {
        opstack.push_back(opstackIn[i]);
    }

    // brackets first:
    bool noBrackets = false;
    while (!noBrackets) {

        //printStack(opstack);

        noBrackets = true;
        int openIndex = -1;
        int closeIndex = -1;
        bool isFunc = false;
        // find innermost set of brackets
        for (unsigned int i = 0; i < opstack.size(); ++i) {

            if (opstack[i].op == BRACKET && opstack[i].val == 1) {
                isFunc = false;
                openIndex = i;
            }
            if (opstack[i].op == FUNC) {
                isFunc = true;
                openIndex = i;
            }
            if (opstack[i].op == BRACKET && opstack[i].val == 2) {
                // throw an error if we haven't had an open bracket, but get a closed
                if (openIndex == -1) return doError(1);
                closeIndex = i;

                float val;
                // if we are a function:
                if (isFunc) {
                    // look for commas
                    int commaIndex = -1;
                    for (unsigned int j = (uint) openIndex; j < (uint) closeIndex; ++j) {
                        if (opstack[j].op == COMMA) {
                            commaIndex = j;
                        }
                    }
                    // if a comma is there...
                    if (commaIndex != -1) {
                        float val1 = doMaths(openIndex+1, commaIndex, opstack);
                        float val2 = doMaths(commaIndex+1, closeIndex, opstack);
                        if (val1 == INFINITY || val2 == INFINITY) return doError(6);
                        if (val1 == -INFINITY || val2 == -INFINITY) return doError(7);
                        val = doFunction(val1, val2, opstack[openIndex].val);
                        if (val == INFINITY) return doError(8);
                    } else {
                        // ...otherwise
                        val = doMaths(openIndex+1, closeIndex, opstack);
                        if (val == INFINITY) return doError(6);
                        if (val == -INFINITY) return doError(7);
                        val = doFunction(val, INFINITY, opstack[openIndex].val);
                        if (val == INFINITY) return doError(8);
                    }

                } else {
                    // otherwise we are good to parse this chunk!
                    val = doMaths(openIndex+1, closeIndex, opstack);
                    if (val == INFINITY) return doError(6);
                    if (val == -INFINITY) return doError(7);
                }

                // put the new value in in place of the brackets:
                opstack.erase(opstack.begin()+openIndex, opstack.begin()+closeIndex+1);
                valop newValOp;
                newValOp.op = VAL;
                newValOp.val = val;
                opstack.insert(opstack.begin() + openIndex, newValOp);

                // still have brackets!
                noBrackets = false;
                // start over
                break;
            }

        }
    }

    // no brackets left, so do rest of maths and send out
    float finalValue = doMaths(0, opstack.size(), opstack);

    if (finalValue == INFINITY) return doError(6);
    if (finalValue == -INFINITY) return doError(7);

    *outVal = finalValue;
    return "";

}

QString doConditional(int startInd, int endInd, vector <valop> opstackIn, float * outVal) {

    vector <valop> opstack;
    for (unsigned int i = (uint) startInd; i < (uint) endInd; ++i) {
        opstack.push_back(opstackIn[i]);
    }

    //printStack(opstack);

    // find the conditional, check there is only one, and then evaluate both sides
    int condInd = -1;
    for (unsigned int i = 0; i < opstack.size(); ++i) {
        if (opstack[i].op == COND) {
            if (condInd != -1) return doError(10);
            condInd = i;
        }
    }

    // if no conditional, then just pass it on
    if (opstack.size() == 1) {

        *outVal = opstack[0].val;

        // success
        return "";

    } else if (condInd == -1) {
        return doError(11);
    }

    // find surrounding brackets
    int brackets = 0;
    for (unsigned int i = 0; i < (uint) condInd; ++i) {
        if ((opstack[i].op == BRACKET && opstack[i].val == 1) || opstack[i].op == FUNC) {
            ++brackets;
        }
        else if (opstack[i].op == BRACKET && opstack[i].val == 2) {
            --brackets;
        }
    }

    if (brackets > 0) {
        opstack.erase(opstack.begin(), opstack.begin()+1);
        opstack.erase(opstack.end()-1, opstack.end());
        --condInd;
    } else if (brackets != 0) {
        return doError(1);
    }

    // evaluate the sides:
    float output1;
    QString err = doBrackets(0, condInd, opstack, &output1);
    if (err != "") return err;

    float output2;
    err = doBrackets(condInd+1, opstack.size(), opstack, &output2);
    if (err != "") return err;

    // do the condition:
    *outVal = doCond(output1, output2, opstack[condInd].val);

    return "";

}

QString doBoolean(int startInd, int endInd, vector <valop> opstackIn, float * outVal)  {

    vector <valop> opstack;
    for (unsigned int i = (uint) startInd; i < (uint) endInd; ++i) {
        opstack.push_back(opstackIn[i]);
    }

    //printStack(opstack);

    // split by the booleans, and evaluate the conditionals in each segment, looking for brackets
    vector <int> boolInds;
    vector <int> boolTypes;

    for (unsigned int i = 0; i < opstack.size(); ++i) {
        if (opstack[i].op == BOOL_OP) {
            boolInds.push_back(i);
            boolTypes.push_back(opstack[i].val);
        }
    }

    vector < float > outs;
    outs.resize(boolInds.size()+1);

    // split and eval each
    for (unsigned int i = 0; i < boolInds.size() + 1; ++i) {

        QString err;
        if (i == 0) {
            err = doConditional(0, boolInds[i], opstack, &outs[i]);
            if (err != "") return err;
        } else if (i == boolInds.size()){
            err = doConditional(boolInds[i-1]+1, opstack.size(), opstack, &outs[i]);
            if (err != "") return err;
        } else {
            err = doConditional(boolInds[i-1]+1, boolInds[i], opstack, &outs[i]);
            if (err != "") return err;
        }

    }

    // now evaluate all the booleans
    *outVal = outs[0];
    for (unsigned int i = 1; i < outs.size(); ++i) {
        if (boolTypes[i-1] == 1) *outVal = *outVal && outs[i];
        if (boolTypes[i-1] == 2) *outVal = *outVal || outs[i];
    }

    return "";

}

QString doBoolBrackets(int startInd, int endInd, vector <valop> opstackIn, float * outVal)  {

    vector <valop> opstack;
    for (unsigned int i = (uint) startInd; i < (uint) endInd; ++i) {
        opstack.push_back(opstackIn[i]);
    }

    // break up into sections, find if brackets are open or closed, then start unwrapping

    bool isStuffToDo = true;

    while (isStuffToDo) {

        isStuffToDo = false;
        vector <int> sectionState;
        vector <int> boolLocs;
        sectionState.push_back(0);

        boolLocs.push_back(-1);
        for (unsigned int i = 0; i < opstack.size(); ++i) {

            if (opstack[i].op == BRACKET && opstack[i].val == 1) {
                sectionState.back() += 1;
            }
            if (opstack[i].op == FUNC) {
                sectionState.back() += 1;
            }
            if (opstack[i].op == BRACKET && opstack[i].val == 2) {
                sectionState.back() -= 1;
            }
            if (opstack[i].op == BOOL_OP) {
                boolLocs.push_back(i);
                sectionState.push_back(0);
            }
        }
        boolLocs.push_back(opstack.size());

        // we now have a list of all the open and closed sections, which must add to zero:
        int sum = 0;
        for (unsigned int i = 0; i < sectionState.size(); ++i) {
            sum += sectionState[i];
        }
        if (sum !=0) return doError(1);

        // now we choose the innermost bools and evaluate them:
        // find max number of open brackets, then find next closed one:
        int openSection = -1;
        int maxOpen = 0;
        sum = 0;
        for (unsigned int i = 0; i < sectionState.size(); ++ i) {
            sum += sectionState[i];
            if (sum > maxOpen) openSection = i;
        }

        // now find the next closed bracket section:
        int closedSection = -1;
        if (openSection != -1) {
            isStuffToDo = true;
            for (unsigned int i = openSection; i < sectionState.size(); ++ i) {
                if (sectionState[i] < 0) {closedSection = i; break;}
            }

            // now do the bools in this section
            float out;
            QString err = doBoolean(boolLocs[openSection]+2, boolLocs[closedSection+1]-1, opstack, &out);
            if (err != "") return err;
            // relace the section with the result
            opstack.erase(opstack.begin()+boolLocs[openSection]+1, opstack.begin()+boolLocs[closedSection+1]);
            valop newValOp;
            newValOp.val = out;
            newValOp.op = VAL;
            opstack.insert(opstack.begin()+boolLocs[openSection]+1, newValOp);

        }

        // if there are no remaining sections with brackets, finish up
        if (openSection == -1) {

            QString err = doBoolean(0, opstack.size(), opstack, outVal);

            if (err != "") return err;

            // *outVal = opstack[0].val;

            // success
            return "";
        }

    }

    return doError(12);

}
*/
float interpretMaths(vector <valop> stack) {

    // evaluate the stack:
    vector <valop> tempStack;

    for (uint i = 0; i < stack.size(); ++i) {

        switch (stack[i].op) {
        case VAL:
            tempStack.push_back(stack[i]);
            /*if (tempStack.back().ptr == NULL)
                qDebug() << "value of " << tempStack.back().val << "\n";
            else
                qDebug() << "pointer to " << *tempStack.back().ptr << "\n";*/
            break;
        case FUNC:
            {
                // fetch first operand from stack
                float val1;
                if (!tempStack.size())
                    val1 = INFINITY;
                else {
                    if (tempStack.back().ptr != NULL)
                        val1 = *tempStack.back().ptr;
                    else
                        val1 = tempStack.back().val;
                    tempStack.pop_back();
                }
                // if stack top is operand then add it in
                float val2;
                if (!stack[i].isUnary) {
                    if (tempStack.size()) {
                        if (tempStack.back().ptr != NULL)
                            val2 = *tempStack.back().ptr;
                        else
                            val2 = tempStack.back().val;
                        tempStack.pop_back();
                    }
                } else {
                    val2 = val1;
                    val1 = INFINITY;
                }

                float result = doFunction(val2, val1, stack[i].val);
                /*qDebug() << "function " << "isUnary(" << float(stack[i].isUnary) << ") " <<  stack[i].val << " " << val2 << " " << val1 << "\n";
                qDebug() << "result = " << result;*/
                // push back result onto stack
                valop res;
                res.val = result;
                res.ptr = NULL;
                res.op = VAL;
                tempStack.push_back(res);
            }
            break;
        case OP:
            {
                // fetch first operand from stack
                float val1;
                if (tempStack.size()) {
                    if (tempStack.back().ptr != NULL)
                        val1 = *tempStack.back().ptr;
                    else
                        val1 = tempStack.back().val;
                    tempStack.pop_back();
                }
                // if stack top is operand then add it in
                float val2 = 0;
                if (!stack[i].isUnary) {
                    if (tempStack.size()) {
                        if (tempStack.back().ptr != NULL)
                            val2 = *tempStack.back().ptr;
                        else
                            val2 = tempStack.back().val;
                        tempStack.pop_back();
                    }
                }
                float result;
                // apply operator (default val2 of 0 handles unary operators)
                switch (int(stack[i].val)) {
                case ADD:
                    result = val2+val1;
                    break;
                case SUB:
                    result = val2-val1;
                    break;
                case MULT:
                    result = val2*val1;
                    break;
                case DIV:
                    result = val2/val1;
                    break;
                }
                // push back result onto stack
               // qDebug() << "operation " << "isUnary(" << float(stack[i].isUnary) << ") " << stack[i].val << " " << val2 << " " << val1 << "\n";
                valop res;
                res.val = result;
                res.ptr = NULL;
                res.op = VAL;
                tempStack.push_back(res);
            }
            break;
        case COND:
            break;
        case BOOL_OP:
            break;
        case LBRACKET:
            break;
        case RBRACKET:
            break;
        case COMMA:
            break;

        }

    }
    //qDebug() << "Stacksize = " << (float) tempStack.size() << "\n";
    if (tempStack.size()) {
        if (tempStack.back().ptr == NULL)
            return tempStack.back().val;
        else
            return *tempStack.back().ptr;
    }

    return 0.0;
}

QString createStack(QString equation, vector <lookup> &varList, vector <valop> * returnStack) {

    vector < valop > opstack;

    // strip all whitespace
    equation.replace(" ", "");

    bool ended = false;

    while (!ended) {

        if (equation.size() == 0) {
            ended = true;
        }

        valop newValOp;
        newValOp.op = COMMA;
        newValOp.val = -1;

        if (isOperation(equation)) {
            newValOp.val = float(getOpVal(equation));
            if (opstack.size() != 0) {
                if (opstack.back().op != VAL && opstack.back().op != RBRACKET)
                    newValOp.isUnary = true;
                else
                    newValOp.isUnary = false;
            } else {
                newValOp.isUnary = true;
            }
            newValOp.op = OP;
            equation.remove(0,1);
        }
        else if (isVar(equation)) {

            QString name = QString(equation[0]);
            equation.remove(0,1);
            // while we are still in the token
            if (equation.size() > 0) {
                while (isToken(equation)) {
                    // get full name
                    name += equation[0];
                    equation.remove(0,1);
                    if (equation.size() == 0) break;
                }
            }

            // fetch the value associated with the name, unless a function
            if (equation.size() > 0) {
                if (equation[0] == '(') {
                    newValOp.val = getFuncVal(name);
                    if (newValOp.val == -1) {qDebug() << "wtf"; return doError(13);}
                    newValOp.isUnary = isFuncUnary(name);
                    newValOp.op = FUNC;
                    equation.remove(0,1);
                }
            }
            if (newValOp.op != FUNC) {
                newValOp.val = INFINITY;//getVarVal(name, varList);
                newValOp.ptr = getVarPtr(name, varList);
                if (newValOp.ptr == NULL) return doError(2);
                newValOp.op = VAL;
            }

        }
        else if (isNum(equation)) {

            QString fullNum = QString(equation[0]);
            equation.remove(0,1);

            if (equation.size() > 0) {
                while (isNum(equation)) {
                    fullNum += equation[0];
                    equation.remove(0,1);
                    if (equation.size() == 0) break;
                }
            }

            newValOp.ptr = NULL;
            newValOp.val = fullNum.toFloat();
            newValOp.op = VAL;

        }
        else if (isCondition(equation)) {

            QString tempCond = QString(equation[0]);
            equation.remove(0,1);

            if (equation.size() > 0) {
                if (isCondition(equation)) {
                    tempCond += equation[0];
                    equation.remove(0,1);
                }
            }

            newValOp.op = COND;
            newValOp.val = float(getCondVal(tempCond));
            if (newValOp.val == ERR) return doError(3);
        }
        else if (equation[0] == '(') {
            newValOp.op = LBRACKET;
            newValOp.val = 1;
            equation.remove(0,1);
        }
        else if (equation[0] == ')') {
            newValOp.op = RBRACKET;
            newValOp.val = 2;
            equation.remove(0,1);
        }
        else if (equation[0] == '&') {
            if (equation.size() < 3) {
                return doError(4);
            }
            newValOp.op = BOOL_OP;
            equation.remove(0,1);
            if (equation[0] != '&') {
                return doError(5);
            }
            newValOp.val = 1;
            equation.remove(0,1);
        }
        else if (equation[0] == '|') {
            if (equation.size() < 3) {
                return doError(4);
            }
            newValOp.op = BOOL_OP;
            equation.remove(0,1);
            if (equation[0] != '|') {
                return doError(5);
            }
            newValOp.val = 2;
            equation.remove(0,1);
        }
        else if (equation[0] == ',') {
            newValOp.op = COMMA;
            newValOp.val = 1;
            equation.remove(0,1);
        }


        opstack.push_back(newValOp);

        //cerr << "INLINE STACK: \n";
        //printStack(opstack);

        //cerr << float(equation.size()) << " " << equation.toStdString() << "\n";

        // finish
        if (equation.size() == 0) {
            ended = true;
        }

    }



    // ok, we have an opstack of symbols - now refactor it for speed:

    /*
    While there are tokens to be read:
    Read a token.
    If the token is a number, then add it to the output queue.
    If the token is a function token, then push it onto the stack.
    If the token is a function argument separator (e.g., a comma):
        Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue. If no left parentheses are encountered, either the separator was misplaced or parentheses were mismatched.
    If the token is an operator, o1, then:
            while there is an operator token, o2, at the top of the stack, and either o1 is left-associative and its precedence is less than or equal to that of o2, or o1 has precedence less than that of o2,
            pop o2 off the stack, onto the output queue;
        push o1 onto the stack.
    If the token is a left parenthesis, then push it onto the stack.
    If the token is a right parenthesis:
        Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
        Pop the left parenthesis from the stack, but not onto the output queue.
        If the token at the top of the stack is a function token, pop it onto the output queue.
        If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
    When there are no more tokens to read:
        While there are still operator tokens in the stack:
            If the operator token on the top of the stack is a parenthesis, then there are mismatched parentheses.
            Pop the operator onto the output queue.
    Exit.
    */

    vector < valop > calcStack;
    vector < valop > tempStack;

    while (opstack.size() > 0) {
        if (opstack[0].op == VAL) {
            calcStack.push_back(opstack[0]);
        }
        else if (opstack[0].op == FUNC) {
            tempStack.push_back(opstack[0]);
        }
        else if (opstack[0].op == COMMA) {
            while (tempStack.back().op != FUNC && tempStack.size() > 0) {
                calcStack.push_back(tempStack.back());
                tempStack.pop_back();
            }
            if (tempStack.size() == 0) return "Error - misplaced ',' or mismatched parentheses";
        }
        else if (opstack[0].op == OP) {
            while (tempStack.size() > 0) {
                if (tempStack.back().op == OP && precedance(opstack[0]) < precedance(tempStack.back())) {
                    calcStack.push_back(tempStack.back());
                    tempStack.pop_back();
                }
                else
                    break;
            }
            tempStack.push_back(opstack[0]);
        }
        else if (opstack[0].op == LBRACKET) {
            tempStack.push_back(opstack[0]);
        }
        else if (opstack[0].op == RBRACKET) {
            if (tempStack.size() == 0) return "Error - mismatched parentheses";
            while (tempStack.back().op != LBRACKET && tempStack.back().op != FUNC) {
                calcStack.push_back(tempStack.back());
                tempStack.pop_back();
            }
            if (tempStack.back().op == LBRACKET) {
                tempStack.pop_back();
            }
            if (tempStack.back().op == FUNC) {
                calcStack.push_back(tempStack.back());
                tempStack.pop_back();
            }
        }
        opstack.erase(opstack.begin());
        //printStack(calcStack);
    }
    // flush stack
    while (tempStack.size() > 0) {
        if (tempStack.back().op == LBRACKET || tempStack.back().op == RBRACKET) return "Error - mismatched backets";
        calcStack.push_back(tempStack.back());
        tempStack.pop_back();
    }

    //cerr << "FINAL RPN STACK: \n";
    //printStack(calcStack);

    // send back

    *returnStack = calcStack;
        // success!
    return "";
}

