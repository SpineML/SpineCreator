#include "NeuronScene.h"
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;

NeuronScene::NeuronScene (QOpenGLShaderProgram *program, QOpenGLFunctions* fns)
{
    this->shaderProgram = program;
    this->f = fns;
}

NeuronScene::~NeuronScene()
{
    this->reset();
}

void
NeuronScene::reset()
{
    DBG() << "NeuronScene: reset (clear this->layers and this->lines)";
    vector<SphereLayer*>::iterator sli = this->layers.begin();
    while (sli != this->layers.end()) {
        delete ((*sli++));
    }
    this->layers.clear();
    vector<LinesLayer*>::iterator lli = this->lines.begin();
    while (lli != this->lines.end()) {
        delete ((*lli++));
    }
    this->lines.clear();
}

SphereLayer*
NeuronScene::createSphereLayer()
{
    DBG() << "NeuronScene: create a new sphere layer";
    SphereLayer* l1 = new SphereLayer(this->shaderProgram);
    this->layers.push_back (l1);
    // Return SphereLayer to caller, ready for them to add spheres to it.
    return l1;
}

void
NeuronScene::initialize()
{
#if 0
    // Some times, create a layer, with parameters (sidelength and z position)
    SphereLayer* l1 = new SphereLayer(this->shaderProgram, 50, 0);
    this->layers.push_back (l1);
    // Then:
    SphereLayer* l2 = new SphereLayer(this->shaderProgram, 50, 0.3);
    this->layers.push_back (l2);

    // Now do the lines using the two layers of spheres
    LinesLayer* ll1 = new LinesLayer(this->shaderProgram, l1, l2);
    this->lines.push_back (ll1);
#endif
}

void
NeuronScene::render()
{
    // for each SphereLayer in this->layers, call render
    vector<SphereLayer*>::iterator sli = this->layers.begin();
    while (sli != this->layers.end()) {
        //DBG() << "NeuronScene: Render some spheres...";
        (*sli)->render (this->f);
        ++sli;
    }
    // Now render each set of lines
    vector<LinesLayer*>::iterator lli = this->lines.begin();
    while (lli != this->lines.end()) {
        //DBG() << "NeuronScene: Render some lines...";
        (*lli)->render (this->f);
        ++lli;
    }
}
