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

#include "dotwriter.h"

DotWriter::DotWriter(RootComponentItem *root)
{
    out = NULL;
    this->root = root;
}

DotWriter::~DotWriter()
{
    if (out != NULL){
        delete out;
        out = NULL;
    }
}

bool DotWriter::writeDotFile(QString file_name)
{
    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        out = new QTextStream(&file);
        *out << "digraph G1 {\n";
        *out << "    graph [fontsize=30 labelloc=\"t\" label=\"NineML Component\" splines=true overlap=false rankdir = \"LR\" nodesep=2.0];\n";
        *out << "    ratio = auto\n";

        writeRegimes();
        writeTransitions();

        *out << "}\n";

        file.close();
        return true;
    }
    else
        return false;
}

void DotWriter::writeRegimes()
{
    for (uint i=0; i<root->al->RegimeList.size(); i++)
    {
        Regime *r = root->al->RegimeList[i];
        *out << "  " << r->name << " [ style = \"filled, bold\"\n";
        *out << "    penwidth = 1\n";
        *out << "    fillcolor = \"white\"\n";
        *out << "    fontname = \"Courier New\"\n";
        *out << "    shape = \"Mrecord\"\n";
        *out << "    label = <<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">\n";
        *out << "            <tr>\n";
        *out << "              <td bgcolor=\"black\" align=\"center\" colspan=\"1\"> <font color=\"white\"> " << r->name << " </font> </td>\n";
        *out << "            </tr>\n";
        for (uint j=0; j< r->TimeDerivativeList.size();j++)
        {
            TimeDerivative *td = r->TimeDerivativeList[j];
            *out << "            <tr>\n";
            *out << "                <td align=\"left\" > d" << td->variable->getName() << "/dt = " << td->maths->getHTMLSafeEquation() << " </td>\n";
            *out << "            </tr>\n";
        }
        *out << "            </table>>];\n";
    }
}

void DotWriter::writeTransitions()
{
    for (uint i=0; i<root->al->RegimeList.size(); i++)
    {
        Regime *r = root->al->RegimeList[i];
        for (uint j=0; j<r->OnConditionList.size(); j++)
        {
            OnCondition *oc = r->OnConditionList[j];
            *out << "  " << r->name <<  " -> " << oc->target_regime->name << "\n";
            *out << "    [ style = \"filled, bold\" \n";
            *out << "    penwidth = 1 \n";
            *out << "    fillcolor = \"white\" \n";
            *out << "    fontname = \"Courier New\" \n";
            *out << "    label = <<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"#C0C0C0\"> \n";
            *out << "            <tr> \n";
            *out << "              <td bgcolor=\"blue\" align=\"center\" > <font color=\"white\"> " << "Transition" << " </font> </td> \n";
            *out << "            </tr> \n";
            *out << "            <tr> \n";
            *out << "              <td bgcolor=\"green\" align=\"center\" > <font color=\"black\"> @ OnCondition&#40; " << oc->trigger->maths->getHTMLSafeEquation() << "&#41; </font> </td> \n";
            *out << "            </tr> \n";
            for (uint k=0; k<oc->StateAssignList.size(); k++)
            {
                StateAssignment *sa = oc->StateAssignList[k];
                *out << "            <tr> \n";
                *out << "              <td> Assign: " << sa->variable->getName() << "&lt;= "<< sa->maths->getHTMLSafeEquation() << " </td> \n";
                *out << "            </tr> \n";
            }
            for (uint k=0; k<oc->eventOutList.size(); k++)
            {
                EventOut * eo = oc->eventOutList[k];
                *out << "            <tr> \n";
                *out << "              <td> Emit Event: " << eo->port->getName() << " </td> \n";
                *out << "            </tr>\n";
            }
            *out << "            </table>> ]; \n";
        }

        for (uint j=0; j<r->OnEventList.size(); j++)
        {
            OnEvent *oe = r->OnEventList[j];
            *out << "  " << r->name <<  " -> " << oe->target_regime->name << "\n";
            *out << "    [ style = \"filled, bold\" \n";
            *out << "    penwidth = 1 \n";
            *out << "    fillcolor = \"white\" \n";
            *out << "    fontname = \"Courier New\" \n";
            *out << "    label = <<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"#C0C0C0\"> \n";
            *out << "            <tr> \n";
            *out << "              <td bgcolor=\"blue\" align=\"center\"> <font color=\"white\"> Transition </font> </td>\n";
            *out << "            </tr> \n";
            *out << "            <tr> \n";
            *out << "              <td bgcolor=\"green\" align=\"center\"> <font color=\"black\"> @ OnEvent&#40; " << oe->src_port->getName() << "&#41; </font> </td> \n";
            *out << "            </tr> \n";
            for (uint k=0; k<oe->StateAssignList.size(); k++)
            {
                StateAssignment *sa = oe->StateAssignList[k];
                *out << "            <tr> \n";
                *out << "              <td> Assign: " << sa->variable->getName() << "&lt;= "<< sa->maths->getHTMLSafeEquation() <<" </td>\n";
                *out << "            </tr> \n";
            }
            for (uint k=0; k<oe->eventOutList.size(); k++)
            {
                EventOut * eo = oe->eventOutList[k];
                *out << "            <tr> \n";
                *out << "              <td>Emit Event: "<< eo->port->getName() << " </td> \n";
                *out << "            </tr>\n";
            }
            *out << "            </table>> ]; \n";
        }
    }
}
