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

#include "globalHeader.h"

// deactivated for now...

versionNumber::versionNumber() {

    // defaults
    major = 0;
    minor = 0;
    revision = 0;
    QSettings settings;
    last_owner = settings.value("versioning/localName", QHostInfo::localHostName()).toString();

}

versionNumber& versionNumber::operator=(const versionNumber& data) {

    major = data.major;
    minor = data.minor;
    revision = data.revision;
    last_owner = data.last_owner;

    return *this;
}

void versionNumber::fromFileString(QString fileName) {

    QStringList strings = fileName.split(".ver");

    // get second string
    if (strings.size() == 2) {
        QStringList bits = strings.at(1).split('_');
        if (bits.size() == 4) {
            major = bits.at(0).toInt();
            minor = bits.at(1).toInt();
            revision = bits.at(2).toInt();
            last_owner = bits.at(3);
            last_owner.chop(4);
            // no need to use defaults
            return;
        }
    }

    // defaults
    major = 0;
    minor = 0;
    revision = 0;
    QSettings settings;
    last_owner = settings.value("versioning/localName", QHostInfo::localHostName()).toString();

}

bool operator==(versionNumber& v1, versionNumber& v2) {

    /*if (v1.major == v2.major)
        if(v1.minor == v2.minor)
            if(v1.revision == v2.revision)
                if(v1.last_owner == v2.last_owner)
                    return true;*/

    return true;
}

bool operator!=(versionNumber& v1, versionNumber& v2) {
    return !(v1==v2);
}

bool operator>=(versionNumber& v1, versionNumber& v2) {

    /*if (v1.last_owner != v2.last_owner)
        return false;
    if (v1.major < v2.major)
        return false;
    if (v1.major == v2.major && v1.minor < v2.minor)
        return false;
    if (v1.major == v2.major && v1.minor == v2.minor && v1.revision < v2.revision)
        return false;*/

    return false;
}

bool operator<=(versionNumber& v1, versionNumber& v2) {

    /*if (v1.last_owner != v2.last_owner)
        return false;
    if (v1.major > v2.major)
        return false;
    if (v1.major == v2.major && v1.minor > v2.minor)
        return false;
    if (v1.major == v2.major && v1.minor == v2.minor && v1.revision > v2.revision)
        return false;

    return true;*/
    return false;
}

bool operator>(versionNumber& v1, versionNumber& v2) {
    return !(v1 <= v2);
}

bool operator<(versionNumber& v1, versionNumber& v2) {
    return !(v1 >= v2);
}


QString versionNumber::toString() {

    return "";
    return "ver: " + QString::number(float(this->major)) + "." + QString::number(float(this->minor)) + "-" + QString::number(float(this->revision));
}

QString versionNumber::toFileString() {

    return "";
    return ".ver" + QString::number(float(this->major)) + "_" + QString::number(float(this->minor)) + "_" + QString::number(float(this->revision)) + "_" + this->last_owner;
}
