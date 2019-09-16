#include "NeuronScene.h"
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

NeuronScene::NeuronScene (QOpenGLShaderProgram *program)
{
    // This call is critical:
    initializeOpenGLFunctions();
    this->shaderProgram = program;
    this->initialize();
}

NeuronScene::~NeuronScene()
{
    vector<SphereLayer*>::iterator sli = this->layers.begin();
    while (sli != this->layers.end()) {
        delete ((*sli++));
    }
    vector<LinesLayer*>::iterator lli = this->lines.begin();
    while (lli != this->lines.end()) {
        delete ((*lli++));
    }
}

void
NeuronScene::initialize (void)
{
    // Some times, create a layer, with parameters (sidelength and z position)
    SphereLayer* l1 = new SphereLayer(this->shaderProgram, 50, 0);
    this->layers.push_back (l1);
    // Then:
    SphereLayer* l2 = new SphereLayer(this->shaderProgram, 50, 0.3);
    this->layers.push_back (l2);

    // Now do the lines using the two layers of spheres
    LinesLayer* ll1 = new LinesLayer(this->shaderProgram, l1, l2);
    this->lines.push_back (ll1);
}

void
NeuronScene::render (void)
{
    // for each SphereLayer in this->layers, call render
    vector<SphereLayer*>::iterator sli = this->layers.begin();
    while (sli != this->layers.end()) {
        (*sli)->render();
        ++sli;
    }
    // Now render each set of lines
    vector<LinesLayer*>::iterator lli = this->lines.begin();
    while (lli != this->lines.end()) {
        (*lli)->render();
        ++lli;
    }
}
