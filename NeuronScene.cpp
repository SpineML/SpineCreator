#include "NeuronScene.h"
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <algorithm>

NeuronScene::NeuronScene (QOpenGLShaderProgram *program, QOpenGLFunctions* fns)
{
    DBG() << "Construct";
    this->shaderProgram = program;
    this->f = fns;
}

NeuronScene::~NeuronScene()
{
    DBG() << "Deconstruct";
    this->reset();
}

void
NeuronScene::reset()
{
    DBG() << "NeuronScene: reset (clear this->layers and this->lines)";
    std::for_each (this->layers.rbegin(), this->layers.rend(), [](const SphereLayer* sli) { delete sli; });
    this->layers.clear();
    std::for_each (this->lines.rbegin(), this->lines.rend(), [](const LinesLayer* lli) { delete lli; });
    this->lines.clear();
}

SphereLayer*
NeuronScene::createSphereLayer()
{
    DBG() << "NeuronScene: create a new sphere layer";
    SphereLayer* l1 = new SphereLayer(this->shaderProgram);
    size_t sz_b4 = this->layers.size();
    this->layers.push_back (l1);
    DBG() << "Sphere layer went from size " << sz_b4 << " to " << this->layers.size();
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
    DBG() << "NeuronScene: Render " << this->layers.size() << " layers in context " << QOpenGLContext::currentContext();
    std::for_each (this->layers.begin(), this->layers.end(), [this](SphereLayer* sli) { sli->render(f); });
    std::for_each (this->lines.begin(), this->lines.end(), [this](LinesLayer* lli) { lli->render(f); });
}
