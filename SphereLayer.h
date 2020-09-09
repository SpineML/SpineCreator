#ifndef SPHERELAYER_H_
#define SPHERELAYER_H_

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>

#include "globalHeader.h"

#include <vector>

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

// Contains the CPU based computations of a layer of spheres as
// triangle vertices. Contains the VBOs for the layer of spheres.
class SphereLayer
{
public:
    SphereLayer (QOpenGLShaderProgram *program);
    SphereLayer (QOpenGLShaderProgram *program, unsigned int sl, float zpos);
    ~SphereLayer();

    //! Do the CPU part of the initialisation - compute vertices etc for a whole square grid of spheres
    void computeSphereGrid();

    //! Compute vertices for, and add a sphere to the SphereLayer model
    void addSphere (const std::vector<float>& posn, const float radius, const std::vector<float>& colour);

    //! Once all spheres have been added, complete the initialization of the model
    void postInit();

    //! Render the sphere layer.
    void render (QOpenGLFunctions* f);

    // Sphere layer attributes
    unsigned int sidelen = 150;
    float zposition = 0.0f;

    //! Number of rings in a sphere
    int rings = 10;
    //! number of segments in one of the sphere rings
    VBOint segments = 12;
    //! sphere radius
    float r = 0.04f;

    //! Something like Spheres1, Spheres2 etc. Or perhaps use Pop name.
    std::string layerName;

    //! A list of the centre coordinates of each sphere in the layer.
    std::vector<loc> sphereCentres;

    //! The model-specific view matrix, used to shift just this layer of spheres wrt to the world
    QMatrix4x4 viewmatrix;
    //! The current 3D offset stored in viewmatrix
    QVector3D offset;

    //! Setter for offset, also updates viewmatrix.
    void setOffset (const QVector3D& _offset)
    {
        this->offset = _offset;
        this->viewmatrix.setToIdentity();
        this->viewmatrix.translate (this->offset);
    }

    //! Shift the offset, also updates viewmatrix.
    void shiftOffset (const QVector3D& _offset)
    {
        this->offset += _offset;
        this->viewmatrix.translate (this->offset);
    }

private:
    // Compute positions and colours of vertices for the sphere and
    // store in these:
    QOpenGLBuffer ivbo;           // Indices Vertex Buffer Object
    QOpenGLBuffer pvbo;           // positions Vertex Buffer Object
    QOpenGLBuffer nvbo;           // normals Vertex Buffer Object
    QOpenGLBuffer cvbo;           // colors Vertex Buffer Object

    // Temporary, internal index
    VBOint idx_int;

    std::vector<VBOint> indices;
    std::vector<float> vertexPositions;
    std::vector<float> vertexNormals;
    std::vector<float> vertexColors;

    QOpenGLShaderProgram* shaderProgram;
public:
    QOpenGLVertexArrayObject vao;
private:
    //! How many spheres in this layer?
    size_t numSpheres = 0;

    //! Set true once postInit is done and all the buffers are set up
    bool postInitDone = false;

    //! Sphere calculation - calculate location of triangle vertices for the sphere. The
    //! sphere will be made up of two "caps" of triangles, and a series of rings.
    //void computeSphere (std::vector<float> positionOffset, VBOint& idx);
    void computeSphere (std::vector<float> positionOffset, VBOint& idx, const std::vector<float>& colour);

    void vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp) const;

    void setupVBO (QOpenGLBuffer& buf, std::vector<float>& dat, const char* arrayname);
};

#endif // SPHERELAYER_H_
