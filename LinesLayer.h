#ifndef _LINESLAYER_H_
#define _LINESLAYER_H_

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>

#include <SphereLayer.h>

#include <vector>
using std::vector;

#include "globalHeader.h" // for struct loc

// Contains the CPU based computations of a layer of liness as
// triangle vertices. Contains the VBOs for the layer of liness.
class LinesLayer
{
public:
    LinesLayer (QOpenGLShaderProgram *program, SphereLayer* sph1, SphereLayer* sph2);
    ~LinesLayer();

    //! Do the CPU part of the initialisation - compute vertices etc.
    void initialize();

    //! Render the lines.
    void render (QOpenGLFunctions* f);

    // Lines layer attributes.
    SphereLayer* sphereLayer1;
    SphereLayer* sphereLayer2;

private:
    // Compute positions and colours of vertices for the sphere and store in these:
    QOpenGLBuffer ivbo;           // Indices Vertex Buffer Object
    QOpenGLBuffer pvbo;           // positions Vertex Buffer Object
    QOpenGLBuffer nvbo;           // normals Vertex Buffer Object
    QOpenGLBuffer cvbo;           // colors Vertex Buffer Object

    vector<VBOint> indices;
    vector<float> vertexPositions;
    vector<float> vertexNormals;
    vector<float> vertexColors;

    QOpenGLShaderProgram* shaderProgram;
    QOpenGLVertexArrayObject vao;

    //! Compute location of three vertices for a triangle that makes up a line.
    void computeLine (const loc& s1, const loc& s2, VBOint& idx);

    void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp);

    void setupVBO (QOpenGLBuffer& buf, vector<float>& dat, const char* arrayname);
};

#endif // _LINESLAYER_H_
