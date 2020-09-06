#ifndef _NEURONSCENE_H_
#define _NEURONSCENE_H_

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>

#include <vector>
using std::vector;

#include "SphereLayer.h"
#include "LinesLayer.h"

// Contains a scene made up of several spherelayers.
class NeuronScene
{
public:
    NeuronScene (QOpenGLShaderProgram *program, QOpenGLFunctions* fns);
    ~NeuronScene();

    void initialize (void);
    void render (void);

    //! The sphere layers.
    vector<SphereLayer*> layers;
    //! The connection layers
    vector<LinesLayer*> lines;

    //! Sphere attributes common to each spherelayer.
    int rings = 6;
    VBOint segments = 8; // number of segments in a ring
    float r = 0.04f;  // sphere radius

private:
    //! The scene holds the shaderProgram
    QOpenGLShaderProgram* shaderProgram;
    //! The scene holds a pointer to the OpenGL functions, which come from the QOpenGLContext.
    QOpenGLFunctions* f;
};

#endif // _NEURONSCENE_H_
