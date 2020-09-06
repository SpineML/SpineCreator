// AVOID overriding paintEvent. The code goes in paintGL.
#if 0
// The code in paintEvent needs splitting up into a "recompute all"
// function, which sets up the vertex buffer objects and recomputes
// all the posititions, and then a render function, which just
// renders. This'll make it fast & easy to change colours of neurons
// etc.
void
glConnectionWidget::paintEvent (QPaintEvent* /*event*/)
{
    // avoid repainting too fast
    if (this->repaintAllowed == false) {
        return;
    } else {
        this->repaintAllowed = false;
        QTimer * timer = new QTimer(this);
        timer->setSingleShot (true);
        connect (timer, SIGNAL(timeout()), this, SLOT(allowRepaint()));
        timer->start(5);
    }

    // don't try and repaint a hidden widget!
    if (!this->isVisible()) {
        return;
    }

    // get rid of old stuff
    if (imageSaveMode) {
        QColor qtCol = QColor::fromRgbF(1.0,1.0,1.0,0.0);
        qglClearColor(qtCol);
    } else {
        QColor qtCol = QColor::fromCmykF(0.5, 0.5, 0.5, 0.0);
        qglClearColor(qtCol.light());
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHT0);
    GLfloat pos0[4] = {-1,-1, 10,0};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    // setup the view
    this->setupView();

    glLoadIdentity();

    // work out scaling for line widths:
    float lineScaleFactor;
    if (imageSaveMode) {
        float maxLen;
        maxLen = imageSaveHeight > imageSaveWidth ? imageSaveHeight : imageSaveWidth;
        lineScaleFactor = (1.0/1000.0*maxLen);
    } else {
        lineScaleFactor = 1.0;
    }

    // add some neurons!

    // fetch quality setting
    QSettings settings;
    int quality = settings.value("glOptions/detail", 5).toInt();

    glPushMatrix();
    glTranslatef(0,0,-5.0);

    // if previewing a layout then override normal drawing
    if (locations.size() > 0) {
        for (int i = 0; i < locations[0].size(); ++i) {
            glPushMatrix();

            glTranslatef(locations[0][i].x, locations[0][i].y, locations[0][i].z);

            // draw with a level of detail dependant on the number on neurons we must draw
            int LoD = round(250.0f/float(locations[0].size())*pow(2,float(quality)));
            // put some bounds on
            if (LoD < 4) {
                LoD = 4;
            }
            if (LoD > 32) {
                LoD = 32;
            }
            this->drawNeuron(0.5, LoD, LoD, QColor(100,100,100,255));

            glPopMatrix();
        }

        glPopMatrix();
        // need this as no painter!
        swapBuffers();
        return;
    }

    // draw with a level of detail dependant on the number on neurons we must draw
    // sum neurons across all pops we'll draw
    int totalNeurons = 0;
    for (int locNum = 0; locNum < selectedPops.size(); ++locNum) {
        totalNeurons += selectedPops[locNum]->layoutType->locations.size();
    }
    int LoD = round(250.0f/float(totalNeurons)*pow(2,float(quality)));

    // normal drawing
    for (int locNum = 0; locNum < selectedPops.size(); ++locNum) {
        QSharedPointer <population> currPop = selectedPops[locNum];
        for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {
            glPushMatrix();

            glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

            // if currently selected
            if (currPop == selectedObject) {
                // move to pop location denoted by the spinboxes for x, y, z
                glTranslatef(loc3Offset.x, loc3Offset.y,loc3Offset.z);
            } else {
                glTranslatef(currPop->loc3.x, currPop->loc3.y,currPop->loc3.z);
            }

            // draw with a level of detail dependant on the number on neurons we must draw
            // put some bounds on
            if (LoD < 4) {
                LoD = 4;
            }
            if (LoD > 32) {
                LoD = 32;
            }
            if (imageSaveMode) {
                LoD = 64;
            }

            // check we haven't broken stuff
            if (popColours[locNum].size() > currPop->layoutType->locations.size()) {
                popColours[locNum].clear();
                popLogs[locNum] = NULL;
            }

            if (popColours[locNum].size() > 0) {
                this->drawNeuron(0.5, LoD, LoD, popColours[locNum][i]);
            } else {
                this->drawNeuron(0.5, LoD, LoD, QColor(100 + 0.5*currPop->colour.red(),
                                                       100 + 0.5*currPop->colour.green(),
                                                       100 + 0.5*currPop->colour.blue(),255));
            }

            glPopMatrix();
        }
    }

    // draw synapses
    for (int targNum = 0; targNum < this->selectedConns.size(); ++targNum) {

        // draw the connections:
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        QSharedPointer <population> src;
        QSharedPointer <population> dst;
        connection * conn;
        QSharedPointer<ComponentInstance> wu;

        if (selectedConns[targNum]->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedConns[targNum]);
            CHECK_CAST(currTarg)
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;

            // Get access to weights, too. For ComponentInstance, see
            // CL_classes.h. A ComponentInstance contains a
            // ParameterInstance, which should contain the weight
            // data.
            wu = currTarg->weightUpdateCmpt;

        } else {
            QSharedPointer<genericInput> currIn = qSharedPointerDynamicCast<genericInput> (selectedConns[targNum]);
            CHECK_CAST(currIn)
            conn = currIn->conn;
            src = qSharedPointerDynamicCast <population> (currIn->source);
            CHECK_CAST(src)
            dst = qSharedPointerDynamicCast <population> (currIn->destination);
            CHECK_CAST(dst)
        }

        float srcX;
        float srcY;
        float srcZ;

        // big offsets for src
        if (src == selectedObject) {
            srcX = this->loc3Offset.x;
            srcY = this->loc3Offset.y;
            srcZ = this->loc3Offset.z;
        } else {
            srcX = src->loc3.x;
            srcY = src->loc3.y;
            srcZ = src->loc3.z;
        }

        float dstX;
        float dstY;
        float dstZ;

        // big offsets for dst
        if (dst == selectedObject) {
            dstX = this->loc3Offset.x;
            dstY = this->loc3Offset.y;
            dstZ = this->loc3Offset.z;
        } else {
            dstX = dst->loc3.x;
            dstY = dst->loc3.y;
            dstZ = dst->loc3.z;
        }

        // check we have the current version of the connectivity
        if (conn->type == CSV) {
            csv_connection * csv_conn = dynamic_cast<csv_connection *> (conn);
            CHECK_CAST(csv_conn)
            if (csv_conn->generator) {
                pythonscript_connection * pyConn = dynamic_cast<pythonscript_connection *> (csv_conn->generator);
                CHECK_CAST(pyConn)
                if (pyConn->changed()) {
                    pyConn->regenerateConnections();
                    // fetch connections back here:
                    connections[targNum].clear();
                    csv_conn->getAllData(connections[targNum]);
                }
            }
        }

        if (conn->type == CSV || conn->type == Python) {

            if (!src->isVisualised && !dst->isVisualised) {
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_LIGHTING);
                continue;
            }

            connGenerationMutex->lock();

            // Only render a few thousand of the black connections lines max
            int inc = 1;
            int connectionsToSkip = 1000; // FIXME: Make this a UI parameter
            if (connections[targNum].size() > connectionsToSkip) {
                // Compute inc based on number of connections:
                inc = (int) connections[targNum].size()/connectionsToSkip;
            }
            for (int i = 0; i < connections[targNum].size(); i+=inc) {

                if (connections[targNum][i].src < src->layoutType->locations.size()
                    && connections[targNum][i].dst < dst->layoutType->locations.size()) {

                    glLineWidth(2.0*lineScaleFactor);
                    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);

                    // draw in
                    glBegin(GL_TRIANGLES);
                    if (src->isVisualised && dst->isVisualised) {
                        glVertex3f(src->layoutType->locations[connections[targNum][i].src].x+srcX,
                                   src->layoutType->locations[connections[targNum][i].src].y+srcY,
                                   src->layoutType->locations[connections[targNum][i].src].z+srcZ);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x+dstX,
                                   dst->layoutType->locations[connections[targNum][i].dst].y+dstY,
                                   dst->layoutType->locations[connections[targNum][i].dst].z+dstZ);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x+dstX,
                                   dst->layoutType->locations[connections[targNum][i].dst].y+dstY,
                                   dst->layoutType->locations[connections[targNum][i].dst].z+dstZ+0.05);
                    }
                    if (src->isVisualised && !dst->isVisualised) {
                        glVertex3f(src->layoutType->locations[connections[targNum][i].src].x,
                                   src->layoutType->locations[connections[targNum][i].src].y,
                                   src->layoutType->locations[connections[targNum][i].src].z);
                        glVertex3f(dstX, dstY, dstZ);
                        glVertex3f(dstX, dstY, dstZ+0.01);
                    }
                    if (!src->isVisualised && dst->isVisualised) {
                        glVertex3f(src->loc3.x, src->loc3.y, src->loc3.z);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x,
                                   dst->layoutType->locations[connections[targNum][i].dst].y,
                                   dst->layoutType->locations[connections[targNum][i].dst].z);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x,
                                   dst->layoutType->locations[connections[targNum][i].dst].y,
                                   dst->layoutType->locations[connections[targNum][i].dst].z+0.01);
                    }
                    glEnd();

                } else {
                    // ERR - CONNECTION INDEX OUT OF RANGE
                }
            }

            // draw selected connections on top
            glDisable(GL_DEPTH_TEST);
            if (selectedConns[targNum] == selectedObject) {

                ParameterInstance* theweights = (ParameterInstance*)0;
                if (selectedConns[targNum]->type == synapseObject) {
                    // Then get the weights from the synapseObject's
                    // weightUpdateCmpt.  Returns non-null pointer if
                    // ComponentInstance is a weight update (tag is
                    // LL:WeightUpdate or just WeightUpdate), and has
                    // a Property. Most weight updates have single
                    // property, but if >1 choose the one called "w"
                    theweights = wu->getWeightsParameter();
                }

                // If we have weights, then we have to find the max and min weights for the connection
                double maxweight = std::numeric_limits<double>::min();
                double minweight = std::numeric_limits<double>::max();
                double m = 0;
                double c = 0;
                if (theweights != (ParameterInstance*)0) {
                    //DBG() << "Redetermining minweight/maxweight...";
#pragma omp parallel for
                    for (int i = 0; i < connections[targNum].size(); ++i) {

                        if (connections[targNum][i].src < src->layoutType->locations.size()
                            && connections[targNum][i].dst < dst->layoutType->locations.size()) {

                            if (((int) connections[targNum][i].src == selectedIndex && selectedType == 1)
                                || ((int) connections[targNum][i].dst == selectedIndex && selectedType == 2)) {

                                if (theweights != (ParameterInstance*)0 && i < theweights->value.size()) {

                                    double myweight = theweights->value[i];
                                    if (myweight > maxweight) {
                                        //DBG() << "Setting maxweight to " << myweight;
                                        maxweight = myweight;
                                    }
                                    if (myweight < minweight) {
                                        //DBG() << "Setting minweight to " << myweight;
                                        minweight = myweight;
                                    }
                                }
                            }
                        }
                    }
                    // Compute a linear scaling from minweight to maxweight
                    double rise = 1.0;
                    double run = maxweight - minweight;
                    m = rise/run;
                    c = 1.0 - m * maxweight;
                    //DBG() << "minweight: " << minweight << " maxweight: " << maxweight << " m: " << m << " c: " << c;
                }

                for (int i = 0; i < connections[targNum].size(); ++i) {

                    if (connections[targNum][i].src < src->layoutType->locations.size()
                        && connections[targNum][i].dst < dst->layoutType->locations.size()) {

                        // Default line width/colour
                        glLineWidth(1.0*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 0.0f, 0.1f);

                        // find if selected
                        bool isSelected = false;
                        for (int j = 0; j < (int) selection.count(); ++j) {
                            if (i == (int)selection[j].row()) {
                                glLineWidth(2.0f*lineScaleFactor);
                                glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
                                isSelected = true;
                                break;
                            }
                            if (connections[targNum][i].src == connections[targNum][selection[j].row()].src && selection[j].column() == 0) {
                                glLineWidth(1.5f*lineScaleFactor);
                                glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
                                isSelected = true;
                            }
                            if (connections[targNum][i].dst == connections[targNum][selection[j].row()].dst && selection[j].column() == 1) {
                                glLineWidth(1.5f*lineScaleFactor);
                                glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
                                isSelected = true;
                            }
                        }
                        float normweight = 1.0f;
                        // This selects the colour of the connections between neural populations in Python/CSV
                        if (((int) connections[targNum][i].src == selectedIndex && selectedType == 1)
                            || ((int) connections[targNum][i].dst == selectedIndex && selectedType == 2)) {

                            if (theweights != (ParameterInstance*)0) {
                                //DBG() << "i=" << i << " and theweights->value.size()=" << theweights->value.size();
                                if (i < theweights->value.size()) {
                                    double myweight = theweights->value[i];
                                    //DBG() << "Weight for this connection line is " << myweight << " m is " << m << " and c is " << c;
                                    normweight = static_cast<float>(m * myweight + c);
                                    //DBG() << "normalised weight is " << normweight;
                                }
                            }
                            glLineWidth(1.5f*lineScaleFactor);

                            // Scale colour on the weight. Perhaps use
                            // two colour maps depending on whether
                            // the minimum weight was >=0 or <0?  NB:
                            // Be sure to set the colour in the next
                            // call to drawNeuron()...
                            glColor4f(normweight, 0.0f, 1.0f-normweight, 1.0f); // Blue to Red. Or blue to yellow?

                            isSelected = true;
                        }

                        if (isSelected) {
                            // Determine starting and final locations of connection lines
                            float strtX = 0.0f, strtY = 0.0f, strtZ = 0.0f;
                            float finX = 0.0f, finY = 0.0f, finZ = 0.0f;
                            if (src->isVisualised && dst->isVisualised) {
                                strtX = src->layoutType->locations[connections[targNum][i].src].x+srcX;
                                strtY = src->layoutType->locations[connections[targNum][i].src].y+srcY;
                                strtZ = src->layoutType->locations[connections[targNum][i].src].z+srcZ;
                                finX = dst->layoutType->locations[connections[targNum][i].dst].x+dstX;
                                finY = dst->layoutType->locations[connections[targNum][i].dst].y+dstY;
                                finZ = dst->layoutType->locations[connections[targNum][i].dst].z+dstZ;

                            } else if (src->isVisualised && !dst->isVisualised) {
                                strtX = src->layoutType->locations[connections[targNum][i].src].x;
                                strtY = src->layoutType->locations[connections[targNum][i].src].y;
                                strtZ = src->layoutType->locations[connections[targNum][i].src].z;
                                finX = dstX;
                                finY = dstY;
                                finZ = dstZ;

                            } else if (!src->isVisualised && dst->isVisualised) {
                                strtX = src->loc3.x;
                                strtY = src->loc3.y;
                                strtZ = src->loc3.z;
                                finX = dst->layoutType->locations[connections[targNum][i].dst].x;
                                finY = dst->layoutType->locations[connections[targNum][i].dst].y;
                                finZ = dst->layoutType->locations[connections[targNum][i].dst].z;
                            }

                            // draw in
                            glBegin(GL_LINES);
                            glVertex3f (strtX, strtY, strtZ);
                            glVertex3f (finX, finY, finZ);
                            glEnd();

                            // Also draw spheres at end of lines to help identify weight map better
                            // Want to translate to finX, finY, finZ then draw the sphere...
                            glPushMatrix();
                            // if source:
                            if (selectedType == 1) {
                                glTranslatef (finX, finY, finZ);
                            } else if (selectedType == 2) {
                                glTranslatef (strtX, strtY, strtZ);
                            }
                            this->drawNeuron (0.5, LoD, LoD, QColor(int(255 * (normweight)),
                                                                    0,
                                                                    int(255 * (1.0f-normweight)), 155)); // Not fully opaque
                            glPopMatrix();
                        }

                    } else {
                        // ERR - CONNECTION INDEX OUT OF RANGE
                    }
                }
            }
            glEnable(GL_DEPTH_TEST);
            connGenerationMutex->unlock();
        }

        if (conn->type == OnetoOne) {

            if (src->numNeurons == dst->numNeurons) {

                if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {

                    for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 1.0f, 0.8f);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dst->layoutType->locations[i].x+dstX, dst->layoutType->locations[i].y+dstY, dst->layoutType->locations[i].z+dstZ);
                        glEnd();
                    }
                }

                if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {

                    for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5f*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 1.0f, 0.8f);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dstX, dstY, dstZ);
                        glEnd();
                    }
                }

                if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {

                    for (int i = 0; i < dst->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5f*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 1.0f, 0.8f);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(srcX, srcY, srcZ);
                        glVertex3f(dst->layoutType->locations[i].x+dst->loc3.x, dst->layoutType->locations[i].y+dst->loc3.y, dst->layoutType->locations[i].z+dst->loc3.z);
                        glEnd();
                    }
                }
            }
        }

        if (conn->type == AlltoAll) {

            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {

                for (int i = 0; i < src->layoutType->locations.size(); ++i) {
                    for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {

                        glLineWidth(1.5f*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 1.0f, 0.2f);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                        glEnd();
                    }
                }
            }
            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {

                for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                    glLineWidth(1.5f*lineScaleFactor);
                    glColor4f(0.0f, 0.0f, 1.0f, 0.2f);
                    // draw in
                    glBegin(GL_LINES);
                    glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                    glVertex3f(dstX, dstY, dstZ);
                    glEnd();
                }
            }
            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {

                 for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {

                    glLineWidth(1.5f*lineScaleFactor);
                    glColor4f(0.0f, 0.0f, 1.0f, 0.2f);
                    // draw in
                    glBegin(GL_LINES);
                    glVertex3f(srcX, srcY, srcZ);
                    glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                    glEnd();
                }
            }
        }

        if (conn->type == FixedProb) {

            fixedProb_connection * fpConn = dynamic_cast <fixedProb_connection *> (conn);
            CHECK_CAST(fpConn)

            random.setSeed(fpConn->seed);

            prob = fpConn->p;

            // generate a list of projections to highlight
            QVector < loc > redrawLocs;

            for (int i = 0; i < src->layoutType->locations.size(); ++i) {
                for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {
                    if (random.value() < this->prob) {
                        glLineWidth(1.0f*lineScaleFactor);
                        glColor4f(0.0f, 0.0f, 0.0f, 0.1f);

                        if (((int) i == selectedIndex && selectedType == 1) \
                                || ((int) j == selectedIndex && selectedType == 2))
                        {
                            // store for redraw of selected connections
                            loc pstart;
                            loc pend;

                            if ((src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) \
                                    || (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0)) {
                                pstart.x = src->layoutType->locations[i].x+srcX;
                                pstart.y = src->layoutType->locations[i].y+srcY;
                                pstart.z = src->layoutType->locations[i].z+srcZ;
                            }
                            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {
                                pstart.x = srcX;
                                pstart.y = srcY;
                                pstart.z = srcZ;
                            }
                            if ((src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) \
                                    || (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0)) {
                                pend.x = dst->layoutType->locations[j].x+dstX;
                                pend.y = dst->layoutType->locations[j].y+dstY;
                                pend.z = dst->layoutType->locations[j].z+dstZ;
                            }
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {
                                pend.x = dstX;
                                pend.y = dstY;
                                pend.z = dstZ;
                            }
                            redrawLocs.push_back(pstart);
                            redrawLocs.push_back(pend);
                        }
                        else
                        {
                            // draw in
                            glBegin(GL_LINES);
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {
                                glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                                glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                            }
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {
                                glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                                glVertex3f(dstX, dstY, dstZ);
                            }
                            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {
                                glVertex3f(srcX, srcY, srcZ);
                                glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                            }
                            glEnd();
                        }
                    }
                }
            }
            // redraw selected (over the top of everything else so no depth test):
            glDisable(GL_DEPTH_TEST);

            for (int i=0; i < redrawLocs.size(); i+=2) {

                // redraw
                glLineWidth(1.5f*lineScaleFactor);
                glColor4f(0.0f, 0.0f, 1.0f, 0.8f);
                glBegin(GL_LINES);
                glVertex3f(redrawLocs[i].x, redrawLocs[i].y,redrawLocs[i].z);
                glVertex3f(redrawLocs[i+1].x, redrawLocs[i+1].y,redrawLocs[i+1].z);
                glEnd();
            }
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);

    glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_MODELVIEW);

    if (popIndicesShown) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen = painter.pen();
        QPen oldPen = pen;
        pen.setColor(QColor(0,0,0,255));
        painter.setPen(pen);

        float zoomVal = zoomFactor;
        if (zoomVal < 0.3f)
            zoomVal = 0.3f;

        // draw text
        for (int locNum = 0; locNum < selectedPops.size(); ++locNum) {
            QSharedPointer <population> currPop = selectedPops[locNum];
            for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {
                glPushMatrix();

                glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

                // if currently selected
                if (currPop == selectedObject) {
                    // move to pop location denoted by the spinboxes for x, y, z
                    glTranslatef(loc3Offset.x, loc3Offset.y,loc3Offset.z);
                } else {
                    glTranslatef(currPop->loc3.x, currPop->loc3.y,currPop->loc3.z);
                }

                // print up text:
                GLdouble modelviewMatrix[16];
                GLdouble projectionMatrix[16];
                GLint viewPort[4];
                GLdouble winX;
                GLdouble winY;
                GLdouble winZ;
                glGetIntegerv(GL_VIEWPORT, viewPort);
                glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
                glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
                gluProject(0, 0, 0, modelviewMatrix, projectionMatrix, viewPort, &winX, &winY, &winZ);

                winX /= RETINA_SUPPORT;
                winY /= RETINA_SUPPORT;

                if (orthoView) {
                    winX += this->width()/4.0;
                    winY -= this->height()/4.0;
                }

                if (imageSaveMode) {
                    //painter.drawText(QRect(winX-(1.0-winZ)*220-20,imageSaveHeight-winY-(1.0-winZ)*220-10,40,20),QString::number(float(i)));
                } else {
                    if (orthoView) {
                        painter.drawText(QRect(winX-(1.0-winZ)*220-10.0/zoomVal-10,
                                               this->height()-winY-(1.0-winZ)*220-10.0/zoomVal-10,40,20),
                                         QString::number(float(i)));
                    } else {
                        painter.drawText(QRect(winX-(1.0-winZ)*300-10.0/zoomVal,
                                               this->height()-winY-(1.0-winZ)*300-10.0/zoomVal,40,20),
                                         QString::number(float(i)));
                    }
                }
                glPopMatrix();
            }
        }
        painter.setPen(oldPen);
        painter.end();
    } else {
        // if the painter isn't there this doesn't get called!
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.end();
    }

    glPopMatrix();
}
#endif // Old paintEvent implementation






// Becomes two fns: setupModel() and then setPerspective(). The former
// creates spheres and so on. The latter computes a orthographic or
// perspective projection view. Most of the setupView() code
#if 0
void
glConnectionWidget::setupView()
{
    DBG() << "Called";
    if (this->imageSaveMode) {
        this->width = this->imageSaveWidth;
        this->height = this->imageSaveHeight;
    } else {
        this->width = this->width()*RETINA_SUPPORT;
        this->height = this->height()*RETINA_SUPPORT;
    }
    glViewport (0, 0, this-width, this->height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    // move view
    if (!orthoView)
        gluPerspective(60.0,((GLfloat)width)/((GLfloat)height), 1.0, 100000.0);
    else {
        float scale = zoomFactor*10.0;
        float aspect = ((GLfloat)height)/((GLfloat)width);
        glOrtho(-scale, scale, -scale*aspect, scale*aspect, -100, 100000.0);
    }

    // preview mode
    if (locations.size() > 0) {

        // Find the maximum extents of the locations.
        loc maxes;
        loc mins;

        maxes.x = -INFINITY; maxes.y = -INFINITY; maxes.z = -INFINITY;
        mins.x = INFINITY; mins.y = INFINITY; mins.z = INFINITY;

        for (int locNum = 0; locNum < locations[0].size(); ++locNum) {
            if (locations[0][locNum].x > maxes.x) {
                maxes.x = locations[0][locNum].x;
            }
            if (locations[0][locNum].y > maxes.y) {
                maxes.y = locations[0][locNum].y;
            }
            if (locations[0][locNum].z > maxes.z) {
                maxes.z = locations[0][locNum].z;
            }
            if (locations[0][locNum].x < mins.x) {
                mins.x = locations[0][locNum].x;
            }
            if (locations[0][locNum].y < mins.y) {
                mins.y = locations[0][locNum].y;
            }
            if (locations[0][locNum].z < mins.z) {
                mins.z = locations[0][locNum].z;
            }
        }

        // now we have max and min for each direction, calculate a view that takes everything in...
        // find the max length of an edge:
        loc lengths;
        lengths.x = maxes.x - mins.x;
        lengths.y = maxes.y - mins.y;
        lengths.z = maxes.z - mins.z;

        float maxLen = lengths.x;
        if (lengths.y > maxLen) { maxLen = lengths.y; }
        if (lengths.z > maxLen) { maxLen = lengths.z; }

        // scale based on max length
        glTranslatef(0,0,-maxLen*1.3);

        // rotate
        glRotatef(-45.0f, 1.0f,0.0f,0.0f);
        glTranslatef(0.0f,2.5f,2.5f);
        glRotatef(45.0f, 0.0f,0.0f,1.0f);

        // translate to centre
        loc centres;
        centres.x = (maxes.x + mins.x) / 2.0f;
        centres.y = (maxes.y + mins.y) / 2.0f;
        centres.z = (maxes.z + mins.z) / 2.0f;

        glTranslatef(-centres.x,-centres.y,-centres.z);

        glMatrixMode(GL_MODELVIEW);

        return; // Case that there is location data

    } else if (selectedPops.size() > 0) {

        // default to 1st pop
        int selIndex = 0;

        // find selected object
        if (selectedObject == NULL) {
            selIndex = 0;
        }
        else {
            for (int i = 0; i < selectedPops.size(); ++i) {
                if (selectedObject == selectedPops[i])
                    selIndex = i;
            }
        }

        QSharedPointer <population> currPop = selectedPops[selIndex];

        // work out extent of view:
        // find max and min vals in each direction:
        loc maxes;
        loc mins;

        maxes.x = -INFINITY; maxes.y = -INFINITY; maxes.z = -INFINITY;
        mins.x = INFINITY; mins.y = INFINITY; mins.z = INFINITY;

        for (int locNum = 0; locNum < currPop->layoutType->locations.size(); ++locNum) {
            if (currPop->layoutType->locations[locNum].x > maxes.x) {
                maxes.x = currPop->layoutType->locations[locNum].x;
            }
            if (currPop->layoutType->locations[locNum].y > maxes.y) {
                maxes.y = currPop->layoutType->locations[locNum].y;
            }
            if (currPop->layoutType->locations[locNum].z > maxes.z) {
                maxes.z = currPop->layoutType->locations[locNum].z;
            }
            if (currPop->layoutType->locations[locNum].x < mins.x) {
                mins.x = currPop->layoutType->locations[locNum].x;
            }
            if (currPop->layoutType->locations[locNum].y < mins.y) {
                mins.y = currPop->layoutType->locations[locNum].y;
            }
            if (currPop->layoutType->locations[locNum].z < mins.z) {
                mins.z = currPop->layoutType->locations[locNum].z;
            }
        }

        // now we have max and min for each direction, calculate a view that takes everything in...
        // find the max length of an edge:
        loc lengths;
        lengths.x = maxes.x - mins.x;
        lengths.y = maxes.y - mins.y;
        lengths.z = maxes.z - mins.z;

        //int maxDir = 0;
        float maxLen = lengths.x;
        if (lengths.y > maxLen) {maxLen = lengths.y; /*maxDir = 1;*/}
        if (lengths.z > maxLen) {maxLen = lengths.z; /*maxDir = 2;*/}

        // scale based on max length
        glTranslatef(0,0,-maxLen*1.2*zoomFactor);
        glTranslatef(pos.x(),pos.y(),0);

        // rotate
        glRotatef(-45.0f+rot.y(), 1.0f,0.0f,0.0f);
        glTranslatef(0.0f,2.5f,2.5f);
        glRotatef(45.0f+rot.x(), 0.0f,0.0f,1.0f);

        // translate to centre
        loc centres;
        centres.x = (maxes.x + mins.x) / 2.0f;
        centres.y = (maxes.y + mins.y) / 2.0f;
        centres.z = (maxes.z + mins.z) / 2.0f;

        glTranslatef(-centres.x,-centres.y,-centres.z);
        glMatrixMode(GL_MODELVIEW);
        return;
    }

    glTranslatef(0.0f,0.0f,-10.0f*zoomFactor);
    glTranslatef(pos.x(),0.0f,0.0f);
    glTranslatef(0.0f,pos.y(),0.0f);
    glRotatef(-45.0f, 1.0f,0.0f,0.0f);
    glRotatef(45.0f, 0.0f,0.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);
}
#endif
