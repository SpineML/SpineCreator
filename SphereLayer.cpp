#include <QtCore/qmath.h>
#include "SphereLayer.h"
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

SphereLayer::SphereLayer(QOpenGLShaderProgram* program)
    : ivbo(QOpenGLBuffer::IndexBuffer)
    , pvbo(QOpenGLBuffer::VertexBuffer)
    , nvbo(QOpenGLBuffer::VertexBuffer)
    , cvbo(QOpenGLBuffer::VertexBuffer)
{
    DBG() << "SphereLayer(QOpenGLShaderProgram*)";
    this->shaderProgram = program;
}

SphereLayer::SphereLayer(QOpenGLShaderProgram* program, unsigned int sl, float zpos)
    : ivbo(QOpenGLBuffer::IndexBuffer)
    , pvbo(QOpenGLBuffer::VertexBuffer)
    , nvbo(QOpenGLBuffer::VertexBuffer)
    , cvbo(QOpenGLBuffer::VertexBuffer)
{
    DBG() << "SphereLayer(QOpenGLShaderProgram*, uint, float)";
    this->shaderProgram = program;
    this->sidelen = sl;
    this->zposition = zpos;
    this->computeSphereGrid();
    this->postInit();
}

SphereLayer::~SphereLayer()
{
    DBG() << "SphereLayer destructor. Context: " << QOpenGLContext::currentContext();

    if (this->postInitDone == false) {
        DBG() << "Nothing was allocated, so just return.";
        return;
    }

    // 0. clear sphereCentres and any other CPU-side member attributes
    this->sphereCentres.clear();
    // 1. bind vertex array object
    this->vao.bind();
    // 2. Release all vertex buffer objects
#if 0
    this->cvbo.release();
    this->nvbo.release();
    this->pvbo.release();
#endif
    this->ivbo.release();
    // 3. Destroy all vertex buffers
    this->cvbo.destroy();
    this->nvbo.destroy();
    this->pvbo.destroy();
    DBG() << "Destroy index vertex buffer object: " << &this->ivbo;
    this->ivbo.destroy();

    // 4. Release vertex array object
    this->vao.release();
    // 5. Destroy vertex array object
    DBG() << "Destroy vertex array object: " << this->vao.objectId();
    this->vao.destroy();

#if 0
    if (this->vao.isCreated()) {
        DBG() << "Destroy vertex array object: " << this->vao.objectId();
        this->vao.destroy();
    }
    if (this->numSpheres > 0) {
        this->sphereCentres.clear();
        if (this->ivbo.isCreated()) {
            DBG() << "Destroy index vertex buffer object: " << &this->ivbo;
            this->ivbo.destroy();
        }
        // Note that pvbo, nvbo cvbo are all set to QOpenGLBuffer::StaticDraw
        if (this->pvbo.isCreated()) {
            //this->shaderProgram->disableAttributeArray("position");
            DBG() << "Destroy position vertex buffer object: " << &this->pvbo;
            this->pvbo.destroy();
        }
        if (this->nvbo.isCreated()) {
            //this->shaderProgram->disableAttributeArray("normalin");
            DBG() << "Destroy normals vertex buffer object: " << &this->nvbo;
            this->nvbo.destroy();
        }
        if (this->cvbo.isCreated()) {
            //this->shaderProgram->disableAttributeArray("color");
            DBG() << "Destroy colours vertex buffer object: " << &this->cvbo;
            this->cvbo.destroy();
        }
    }
#endif
}

void SphereLayer::setupVBO (QOpenGLBuffer& buf, vector<float>& dat, const char* arrayname)
{
    if (buf.create() == false) {
        cout << "VBO create failed" << endl;
    }
    buf.setUsagePattern (QOpenGLBuffer::StaticDraw);
    if (buf.bind() == false) {
        cout << "VBO bind failed" << endl;
    }
    buf.allocate (dat.data(), dat.size() * sizeof(float));

    // We have the same shaderProgram for all SphereLayers, but for different
    // spherelayers we have different vertex array objects. How does rendering know to
    // switch between vertex array objects when rendering? Because at each call to
    // render, the vao is bound. So, I prolly need to bind each vertex buffer object
    // too, at the start of the render call.

    // Because array attributes are disabled by default in OpenGL 4:
    this->shaderProgram->enableAttributeArray (arrayname); // Called many times. Is this a problem?
    this->shaderProgram->setAttributeBuffer (arrayname, GL_FLOAT, 0, 3);
}

void SphereLayer::postInit()
{
    QOpenGLContext* ccon = QOpenGLContext::currentContext();
    DBG() << "SphereLayer::postInit: " << ccon;

    this->vao.create(); // Creates VAO on OpenGL server
    this->vao.bind();
    if (this->vao.isCreated() == false) {
        cout << "Uh oh, couldn't bind vao" << endl;
    }
    DBG() << "Created Vertex Array Object " << this->vao.objectId();

    // Index buffer - slightly different from process to setupVBO
    // (because no setAttributeBuffer call)
    if (this->ivbo.create() == false) {
        cout << "ivbo create failed" << endl;
    }
    DBG() << "Created index vertex buffer object: " << &this->ivbo;
    this->ivbo.setUsagePattern (QOpenGLBuffer::StaticDraw);
    if (this->ivbo.bind() == false) {
        cout << "ivbo bind failed" << endl;
    }
    int sz = this->indices.size() * sizeof(VBOint);
    this->ivbo.allocate (this->indices.data(), sz);
    // setAttributeArray or setAttributeBuffer??? I've used _Array_ in setupVBO and _Buffer_ here. ?!?!
    // In working morph code, for indices, I call glBindBuffer/glBufferData(GL_ELEMENT_ARRAY_BUFFER,...)
    // and for setupVBO, I call glBindBuffer/glBufferData(GL_ARRAY_BUFFER,...)
    //
    // setAttrubteBuffer works on the currently bound buffer; whereas setAttributeArray
    // copies in the data at the same time.
    this->shaderProgram->setAttributeBuffer("ebo", VBO_ENUM_TYPE, 0, 1);
    this->shaderProgram->enableAttributeArray("ebo");

    // Binds data from the "C++ world" to the OpenGL shader world for
    // "position", "normalin" and "color" on the currently bound vertex array object
    this->setupVBO (this->pvbo, this->vertexPositions, "position");
    this->setupVBO (this->nvbo, this->vertexNormals, "normalin");
    this->setupVBO (this->cvbo, this->vertexColors, "color");

    // Release (unbind) all vertex buffer objects. Correct place for this?
    // this->ivbo.release(); // but causes problem to release this..
    // FIXME: Should I release pvbo etc?
#if 1
    this->pvbo.release();
    this->nvbo.release();
    this->cvbo.release();
#endif
    this->vao.release();

    this->postInitDone = true;
}

void SphereLayer::render (QOpenGLFunctions* f)
{
    if (this->numSpheres == 0) { return; }
    // The same shaderProgram is used to render each SphereLayer. For it to know what to
    // render, we must bind the relevant vertex array object, which "knows" where the
    // vertex buffer objects are.
    this->vao.bind();
    DBG() << "Render " << this->numSpheres << " spheres (" << this->indices.size() << " vertices) Context: " << QOpenGLContext::currentContext();
    f->glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
    this->vao.release();
}

// pi as a single precision float; used in computeSphere()
#define M_PI_F 3.14159265f

void SphereLayer::computeSphere (vector<float> positionOffset, VBOint& idx, const vector<float>& colour)
{
    // For each computed sphere, save the coordinate of the centre of the sphere.
    this->sphereCentres.push_back (loc (positionOffset[0], positionOffset[1], positionOffset[2]));

    // First cap, draw as a triangle fan, but record indices so that
    // we only need a single call to glDrawElements. NB: each cap is a
    // "ring"
    float rings0 = M_PI_F * -0.5f;
    float _z0  = sin(rings0);
    float z0  = this->r * _z0/* + positionOffset[2]*/;
    float r0 =  cos(rings0);
    float rings1 = M_PI_F * (-0.5f + 1.0f / rings);
    float _z1 = sin(rings1);
    float z1 = r * _z1 /*+ positionOffset[2]*/;
    float r1 = cos(rings1);
    // Push the central point
    this->vertex_push (0.0f + positionOffset[0], 0.0f + positionOffset[1], z0 + positionOffset[2], this->vertexPositions);
    this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
    this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexColors); // Black

    VBOint capMiddle = idx++;
    VBOint ringStartIdx = idx;
    VBOint lastRingStartIdx = idx;

    bool firstseg = true;
    for (unsigned int j = 0; j < segments; j++) {
        float segment = 2 * M_PI * (float) (j) / segments;
        float x = cos(segment);
        float y = sin(segment);

        float _x1 = x*r1;
        float x1 = _x1*r;
        float _y1 = y*r1;
        float y1 = _y1*r ;

        this->vertex_push (x1 + positionOffset[0], y1 + positionOffset[1], z1 + positionOffset[2], this->vertexPositions);
        this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
#ifdef BEACHBALL
        if (j%2) {
            this->vertex_push (1.0f, 0.2f, 0.0f, this->vertexColors);
        } else {
            this->vertex_push (0.0f, 0.2f, 1.0f, this->vertexColors);
        }
#else
        this->vertex_push (colour[0], colour[1], colour[2], this->vertexColors);
#endif

        if (!firstseg) {
            this->indices.push_back (capMiddle);
            this->indices.push_back (idx-1);
            this->indices.push_back (idx++);
        } else {
            idx++;
            firstseg = false;
        }
    }
    this->indices.push_back (capMiddle);
    this->indices.push_back (idx-1);
    this->indices.push_back (capMiddle+1);

    // Now add the triangles around the rings
    for (int i = 2; i < rings; i++) {

        rings0 = M_PI * (-0.5 + (float) (i) / rings);
        _z0  = sin(rings0);
        z0  = r * _z0/* + positionOffset[2]*/;
        r0 =  cos(rings0);

        for (unsigned int j = 0; j < segments; j++) {

            // "current" segment
            float segment = 2 * M_PI * (float)j / segments;
            float x = cos(segment);
            float y = sin(segment);

            // One vertex per segment
            float _x0 = x*r0;
            float x0 = _x0*r/* + positionOffset[0]*/;
            float _y0 = y*r0;
            float y0 = _y0*r/* + positionOffset[1]*/;

            // NB: Only add ONE vertex per segment. ALREADY have the first ring!
            this->vertex_push (x0 + positionOffset[0], y0 + positionOffset[1], z0 + positionOffset[2], this->vertexPositions);
            // The vertex normal of a vertex that makes up a sphere is
            // just a normal vector in the direction of the vertex.
            this->vertex_push (_x0, _y0, _z0, this->vertexNormals);
#ifdef BEACHBALL
            if (j%2) {
                this->vertex_push (1.0f, 0.2f, 0.0f, this->vertexColors);
            } else {
                this->vertex_push (0.0f, 0.2f, 1.0f, this->vertexColors);
            }
#else
            this->vertex_push (colour[0], colour[1], colour[2], this->vertexColors);
#endif
            if (j == (segments - 1)) {
                // Last vertex is back to the start
                this->indices.push_back (ringStartIdx++);
                this->indices.push_back (idx);
                this->indices.push_back (lastRingStartIdx);
                this->indices.push_back (lastRingStartIdx);
                this->indices.push_back (idx++);
                this->indices.push_back (lastRingStartIdx+segments);
            } else {
                this->indices.push_back (ringStartIdx++);
                this->indices.push_back (idx);
                this->indices.push_back (ringStartIdx);
                this->indices.push_back (ringStartIdx);
                this->indices.push_back (idx++);
                this->indices.push_back (idx);
            }
        }
        lastRingStartIdx += segments;
    }

    // bottom cap
    rings0 = M_PI * 0.5;
    _z0  = sin(rings0);
    z0  = r * _z0/* + positionOffset[2]*/;
    r0 =  cos(rings0);
    // Push the central point of the bottom cap
    this->vertex_push (0.0f + positionOffset[0], 0.0f + positionOffset[1], z0 + positionOffset[2], this->vertexPositions);
    this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
    this->vertex_push (1.0f, 1.0f, 1.0f, this->vertexColors); // White
    capMiddle = idx++;
    firstseg = true;
    // No more vertices to push, just do the indices for the bottom cap
    ringStartIdx = lastRingStartIdx;
    for (unsigned int j = 0; j < segments; j++) {
        if (j != (segments - 1)) {
            this->indices.push_back (capMiddle);
            this->indices.push_back (ringStartIdx++);
            this->indices.push_back (ringStartIdx);
        } else {
            // Last segment
            this->indices.push_back (capMiddle);
            this->indices.push_back (ringStartIdx);
            this->indices.push_back (lastRingStartIdx);
        }
    }

    // end of sphere calculation
    //cout << "Number of vertexPositions coords: " << (this->vertexPositions.size()/3) << endl;
}

void SphereLayer::addSphere (const vector<float>& posn, const float radius, const vector<float>& colour)
{
    this->r = radius;
    DBG() << "Add sphere at position (" << posn[0] << "," << posn[1] << "," << posn[2] << ")";
    this->computeSphere (posn, this->idx_int, colour);
    this->numSpheres++;
}

void SphereLayer::computeSphereGrid (void)
{
    // Spacing is set up here. Probably need to have a constructor
    // which passes in spacing, dims or even specific locations of
    // each sphere (neuron)
    float spacing = 0.1f;

    vector<float> po = {{ -(this->sidelen/2.0f*spacing), -(this->sidelen/2.0f*spacing), this->zposition }};
    VBOint idx = 0;

    for (unsigned int a = 0; a < this->sidelen; a++) {
        po[0] = -2.5f;
        for (unsigned int b = 0; b < this->sidelen; b++) {
            this->computeSphere (po, idx, {0.2f, 0.2f, 0.5f});
            po[0] += spacing;
        }
        po[1] += spacing;
    }
    cout << "After compute sphere " << (this->sidelen*this->sidelen) << " times, we have "
         << (this->vertexPositions.size()/3) << " vertex coordinates" << endl;
}

void SphereLayer::vertex_push (const float& x, const float& y, const float& z, vector<float>& vp) const
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}
