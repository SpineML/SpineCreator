#include "NeuronScene.h"
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <algorithm>


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
NeuronScene::render()
{
    DBG() << "NeuronScene: Render " << this->layers.size() << " layers in context " << QOpenGLContext::currentContext();
    std::for_each (this->layers.begin(), this->layers.end(), [this](SphereLayer* sli) { sli->render(f); });
    std::for_each (this->lines.begin(), this->lines.end(), [this](LinesLayer* lli) { lli->render(f); });
}
