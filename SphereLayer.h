#ifndef SPHERELAYER_H_
#define SPHERELAYER_H_

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include "globalHeader.h"
#include "NL_population.h"

#include <vector>

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

// Contains the CPU based computations of a layer of spheres as
// triangle vertices. Contains the VBOs for the layer of spheres.
class SphereLayer
{
public:
    SphereLayer (QOpenGLShaderProgram *program)
    : ivbo(QOpenGLBuffer::IndexBuffer)
    , pvbo(QOpenGLBuffer::VertexBuffer)
    , nvbo(QOpenGLBuffer::VertexBuffer)
    , cvbo(QOpenGLBuffer::VertexBuffer)
    {
        this->shaderProgram = program;
    }

    SphereLayer (QOpenGLShaderProgram *program, unsigned int sl, float zpos)
    : ivbo(QOpenGLBuffer::IndexBuffer)
    , pvbo(QOpenGLBuffer::VertexBuffer)
    , nvbo(QOpenGLBuffer::VertexBuffer)
    , cvbo(QOpenGLBuffer::VertexBuffer)
    {
        this->shaderProgram = program;
        this->sidelen = sl;
        this->zposition = zpos;
        this->computeSphereGrid();
        this->postInit();
    }

    ~SphereLayer()
    {
        if (this->postInitDone == false) {
            DBG() << "Nothing was allocated, so just return.";
            return;
        }
        // 0. clear sphereCentres and any other CPU-side member attributes
        this->sphereCentres.clear();
        // 1. bind vertex array object
        // this->vao.bind();
        // 2. Release all vertex buffer objects
        //this->cvbo.release();
        //this->nvbo.release();
        //this->pvbo.release();
        this->ivbo.release();
        // 3. Destroy all vertex buffers
        this->cvbo.destroy();
        this->nvbo.destroy();
        this->pvbo.destroy();
        DBG() << "Destroy index vertex buffer object: " << &this->ivbo;
        this->ivbo.destroy();
        // 4. Release vertex array object
        // this->vao.release();
        // 5. Destroy vertex array object
        DBG() << "Destroy vertex array object: " << this->vao.objectId();
        this->vao.destroy();
    }

    //! Do the CPU part of the initialisation - compute vertices etc for a whole square grid of spheres
    void computeSphereGrid();

    //! Compute vertices for, and add a sphere to the SphereLayer model
    void addSphere (const std::vector<float>& posn, const float radius, const std::vector<float>& colour)
    {
        this->r = radius;
        DBG() << "Add sphere at position (" << posn[0] << "," << posn[1] << "," << posn[2] << ")";
        this->computeSphere (posn, this->idx_int, colour);
        this->numSpheres++;
    }
    //! Once all spheres have been added, complete the initialization of the model
    void postInit()
    {
        this->vao.create(); // Creates VAO on OpenGL server
        this->vao.bind();
        if (this->vao.isCreated() == false) { cout << "Uh oh, couldn't bind vao" << endl; }
        // Index buffer setup is slightly different from that setupVBO
        if (this->ivbo.create() == false) { cout << "ivbo create failed" << endl; }
        this->ivbo.setUsagePattern (QOpenGLBuffer::StaticDraw);
        if (this->ivbo.bind() == false) { cout << "ivbo bind failed" << endl; }
        int sz = this->indices.size() * sizeof(VBOint);
        this->ivbo.allocate (this->indices.data(), sz);
        this->shaderProgram->setAttributeBuffer("ebo", VBO_ENUM_TYPE, 0, 1);
        this->shaderProgram->enableAttributeArray("ebo");
        // Binds data from the "C++ world" to the OpenGL shader world for "position",
        // "normalin" and "color" on the currently bound vertex array object
        this->setupVBO (this->pvbo, this->vertexPositions, "position");
        this->setupVBO (this->nvbo, this->vertexNormals, "normalin");
        this->setupVBO (this->cvbo, this->vertexColors, "color");

#if 1   // Releasing pvbo etc here seems to work and is how I do it in morphologica.
        // Release (unbind) all vertex buffer objects. Correct place for this?
        // this->ivbo.release(); // but causes problem to release this..
        this->pvbo.release();
        this->nvbo.release();
        this->cvbo.release();
#endif
        this->vao.release();
        this->postInitDone = true;
    }

    //! Render the sphere layer.
    void render (QOpenGLFunctions* f)
    {
        if (this->numSpheres == 0) { return; }
        // The same shaderProgram is used to render each SphereLayer. For it to know what to
        // render, we must bind the relevant vertex array object, which "knows" where the
        // vertex buffer objects are.
        this->setOffset(this->pop->loc3);
        this->vao.bind();
        f->glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
        this->vao.release();
    }

    //! Number of spheres along a side of the square grid. Used in computeSphereGrid()
    unsigned int sidelen = 150;
    //! The z coordinate of the sphere layer grid. Used in computeSphereGrid()
    float zposition = 0.0f;

    //! Number of rings in a sphere
    int rings = 10;
    //! number of segments in one of the sphere rings
    VBOint segments = 12;
    //! sphere radius
    float r = 0.04f;

    //! A list of the centre coordinates of each sphere in the layer.
    std::vector<loc> sphereCentres;

    //! The model-specific matrix, used to shift just this layer of spheres wrt itself
    QMatrix4x4 modelmatrix;
    //! The current 3D offset stored in modelmatrix
    QVector3D offset;

    //! The population which this SphereLayer is visualizing.
    QSharedPointer<population> pop;

    //! Setter for offset, also updates modelmatrix.
    void setOffset (const loc& _offset)
    {
        this->offset[0] = _offset.x;
        this->offset[1] = _offset.y;
        this->offset[2] = _offset.z;
        this->modelmatrix.setToIdentity();
        this->modelmatrix.translate (this->offset);
    }

    void setOffset (const QVector3D& _offset)
    {
        this->offset = _offset;
        this->modelmatrix.setToIdentity();
        this->modelmatrix.translate (this->offset);
    }

    //! Shift the offset, also updates modelmatrix.
    void shiftOffset (const QVector3D& _offset)
    {
        this->offset += _offset;
        this->modelmatrix.translate (this->offset);
    }

private:
    // Compute positions and colours of vertices for the sphere and store in these:
    QOpenGLBuffer ivbo; //! Indices Vertex Buffer Object
    QOpenGLBuffer pvbo; //! positions Vertex Buffer Object
    QOpenGLBuffer nvbo; //! normals Vertex Buffer Object
    QOpenGLBuffer cvbo; //! colors Vertex Buffer Object

    // A temporary, internal index
    VBOint idx_int = 0;

    // CPU-side data. Holds the vertices that make up spheres as computed by this class.
    std::vector<VBOint> indices;
    std::vector<float> vertexPositions;
    std::vector<float> vertexNormals;
    std::vector<float> vertexColors;

    QOpenGLShaderProgram* shaderProgram;
public: // Temporarily:
    QOpenGLVertexArrayObject vao;
private:
    //! How many spheres in this layer?
    size_t numSpheres = 0;

    //! Set true once postInit is done and all the buffers are set up
    bool postInitDone = false;

    //! Sphere calculation - calculate location of triangle vertices for the sphere. The
    //! sphere will be made up of two "caps" of triangles, and a series of rings.
    void computeSphere (std::vector<float> positionOffset, VBOint& idx, const std::vector<float>& colour);

    void setupVBO (QOpenGLBuffer& buf, std::vector<float>& dat, const char* arrayname)
    {
        if (buf.create() == false) { std::cout << "VBO create failed" << std::endl; }
        buf.setUsagePattern (QOpenGLBuffer::StaticDraw);
        if (buf.bind() == false) { std::cout << "VBO bind failed" << std::endl; }
        buf.allocate (dat.data(), dat.size() * sizeof(float));
        // We have the same shaderProgram for all SphereLayers, but for different
        // spherelayers we have different vertex array objects. How does rendering know to
        // switch between vertex array objects when rendering? Because at each call to
        // render, the vao is bound. So, I prolly need to bind each vertex buffer object
        // too, at the start of the render call.
        //
        // Because array attributes are disabled by default in OpenGL 4:
        this->shaderProgram->enableAttributeArray (arrayname); // Called many times. Is this a problem?
        this->shaderProgram->setAttributeBuffer (arrayname, GL_FLOAT, 0, 3);
    }

    void vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp) const;
};

#endif // SPHERELAYER_H_
