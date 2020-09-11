#pragma once

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>

#include <vector>

#include "SphereLayer.h"
#include "LinesLayer.h"

//typedef GLuint VBOint;
//#define VBO_ENUM_TYPE GL_UNSIGNED_INT

//! The locations for the position, normal and colour vertex attributes in the GLSL program
//enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2 };

//! Contains a scene made up of several spherelayers. Similar to morph::Visual (see
//! Seb's other project, morphologica)
class NeuronScene
{
public:
    NeuronScene (QOpenGLShaderProgram *program, QOpenGLFunctions* fns)
    {
        this->shaderProgram = program;
        this->f = fns;
    }

    ~NeuronScene() { this->reset(); }

    //! Deallocate any lines/spheres
    void reset()
    {
        DBG() << "NeuronScene: reset (clear this->layers and this->lines)";
        std::for_each (this->layers.rbegin(), this->layers.rend(), [](const SphereLayer* sli) { delete sli; });
        this->layers.clear();
        std::for_each (this->lines.rbegin(), this->lines.rend(), [](const LinesLayer* lli) { delete lli; });
        this->lines.clear();
    }

    void render()
    {
        std::for_each (this->layers.begin(), this->layers.end(), [this](SphereLayer* sli) { sli->render(f); });
        std::for_each (this->lines.begin(), this->lines.end(), [this](LinesLayer* lli) { lli->render(f); });
    }

    //! Allocate memory for a new, empty sphere layer.
    SphereLayer* createSphereLayer()
    {
        SphereLayer* l1 = new SphereLayer(this->shaderProgram);
        this->layers.push_back (l1);
        // Return SphereLayer to caller, ready for them to add spheres to it.
        return l1;
    }

    //! The sphere layers.
    std::vector<SphereLayer*> layers;
    //! The connection layers
    std::vector<LinesLayer*> lines;

    //! Sphere attributes common to each spherelayer.
    int rings = 6;
    VBOint segments = 8; // number of segments in a ring
    float r = 0.04f;  // sphere radius

protected:
    //! The scene holds the shaderProgram
    QOpenGLShaderProgram* shaderProgram;
    //! The scene holds a pointer to the OpenGL functions, which come from the QOpenGLContext.
    QOpenGLFunctions* f;
};
