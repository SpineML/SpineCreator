#ifndef SPHERELAYER_H_
#define SPHERELAYER_H_

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_5_Core>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>

#include <vector>
using std::vector;

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

// Replace with loc perhaps
struct coord
{
    coord (float _x, float _y, float _z)
        : x(_x)
        , y(_y)
        , z(_z) {}
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Contains the CPU based computations of a layer of spheres as
// triangle vertices. Contains the VBOs for the layer of spheres.
class SphereLayer : protected QOpenGLFunctions_4_5_Core
{
public:
    SphereLayer (QOpenGLShaderProgram *program);
    SphereLayer (QOpenGLShaderProgram *program, unsigned int sl, float zpos);
    ~SphereLayer();

    /*!
     * Do the CPU part of the initialisation - compute vertices etc.
     */
    void initialize();

    /*!
     * Render the sphere layer.
     */
    void render();

    //! Sphere layer attributes
    //@{
    unsigned int sidelen = 150;
    float zposition = 0.0f;
    //@}

    //! Sphere attributes (hardcoded for now)
    //@{
    int rings = 6;
    VBOint segments = 8; // number of segments in a ring
    float r = 0.04f;  // sphere radius
    //@}

    /*!
     * A list of the centre coordinates of each sphere in the layer.
     */
    vector<coord> sphereCentres;

private:
    // Compute positions and colours of vertices for the sphere and
    // store in these:
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

    // Sphere calculation - calculate location of triangle vertices
    // for the sphere. The sphere will be made up of two "caps" of
    // triangles, and a series of rings.
    void computeSphere (vector<float> positionOffset, VBOint& idx);

    void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp);

    void setupVBO (QOpenGLBuffer& buf, vector<float>& dat, const char* arrayname);
};

#endif // SPHERELAYER_H_
