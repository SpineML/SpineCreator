#include <QtCore/qmath.h>
#include "LinesLayer.h"
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

LinesLayer::LinesLayer(QOpenGLShaderProgram *program, SphereLayer* sph1, SphereLayer* sph2)
    : ivbo(QOpenGLBuffer::IndexBuffer)
    , pvbo(QOpenGLBuffer::VertexBuffer)
    , nvbo(QOpenGLBuffer::VertexBuffer)
    , cvbo(QOpenGLBuffer::VertexBuffer)
{
    this->shaderProgram = program;
    this->sphereLayer1 = sph1;
    this->sphereLayer2 = sph2;
    this->initialize();
}

LinesLayer::~LinesLayer()
{
    if (this->ivbo.isCreated()) {
        this->ivbo.destroy();
    }
    if (this->pvbo.isCreated()) {
        this->pvbo.destroy();
    }
    if (this->nvbo.isCreated()) {
        this->nvbo.destroy();
    }
    if (this->cvbo.isCreated()) {
        this->cvbo.destroy();
    }
}

void
LinesLayer::computeLine (const coord& s1, const coord& s2, VBOint& idx)
{
    float dx = (s2.x - s1.x);
    float dy = (s2.y - s1.y);
    float d = sqrtf (dx*dx + dy*dy);
    //if (d < 0.5f) {
    //    cout << "d=" << d << endl;
    //}
    if (d < 0.2f) { // Only draw lines between 'close' neurons
        this->vertex_push (s1.x, s1.y, s1.z, this->vertexPositions);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexNormals);
        this->vertex_push (1.0f, 0.0f, 0.0f, this->vertexColors);
        this->indices.push_back (idx++);

        this->vertex_push (s2.x, s2.y, s2.z, this->vertexPositions);
        this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexNormals);
        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexColors);
        this->indices.push_back (idx++);
    }
}

void
LinesLayer::initialize (void)
{
    VBOint idx = 0;

    // For each sphere in sphereLayer1, draw lines to all spheres in sphereLayer2
    vector<coord>::const_iterator spl1 = this->sphereLayer1->sphereCentres.begin();
    while (spl1 != this->sphereLayer1->sphereCentres.end()) {
        vector<coord>::const_iterator spl2 = this->sphereLayer2->sphereCentres.begin();
        while (spl2 != this->sphereLayer2->sphereCentres.end()) {
            // Draw triangle from *spl1 to *spl2
            this->computeLine (*spl1, *spl2, idx);
            ++spl2;
        }
        ++spl1;
    }

    this->vao.create();
    this->vao.bind();

    // Index buffer - slightly different from process to setupVBO
    // (because no setAttributeBuffer call)
    if (this->ivbo.create() == false) {
        cout << "ivbo create failed" << endl;
    }
    this->ivbo.setUsagePattern (QOpenGLBuffer::StaticDraw);
    if (this->ivbo.bind() == false) {
        cout << "ivbo bind failed" << endl;
    }
    int sz = this->indices.size() * sizeof(VBOint);
    this->ivbo.allocate (this->indices.data(), sz);
    this->shaderProgram->setAttributeBuffer("ebo", VBO_ENUM_TYPE, 0, 1);
    this->shaderProgram->enableAttributeArray("ebo");

    // Binds data from the "C++ world" to the OpenGL shader world for
    // "position", "normalin" and "color"
    this->setupVBO (this->pvbo, this->vertexPositions, "position");
    this->setupVBO (this->nvbo, this->vertexNormals, "normalin");
    this->setupVBO (this->cvbo, this->vertexColors, "color");

    // Release (unbind) all
    // this->ivbo.release(); // but causes problem to release this..
    this->pvbo.release();
    this->nvbo.release();
    this->cvbo.release();

    this->vao.release();
}

// Note: LinesLayer and SphereLayer could derive from a parent class with VBOs in.
void
LinesLayer::setupVBO (QOpenGLBuffer& buf, vector<float>& dat, const char* arrayname)
{
    if (buf.create() == false) {
        cout << "VBO create failed" << endl;
    }
    buf.setUsagePattern (QOpenGLBuffer::StaticDraw);
    if (buf.bind() == false) {
        cout << "VBO bind failed" << endl;
    }
    buf.allocate (dat.data(), dat.size() * sizeof(float));
    // Because array attributes are disabled by default in OpenGL 4:
    this->shaderProgram->enableAttributeArray (arrayname);
    this->shaderProgram->setAttributeBuffer (arrayname, GL_FLOAT, 0, 3);
}

void
LinesLayer::render(QOpenGLFunctions* f)
{
    this->vao.bind();
    // Make lines thick? large glLineWidth seems to make no difference.
    f->glLineWidth (1.0f);
    f->glDrawElements (GL_LINES, this->indices.size(), VBO_ENUM_TYPE, 0);
    this->vao.release();
}

void
LinesLayer::vertex_push (const float& x, const float& y, const float& z, vector<float>& vp)
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}
