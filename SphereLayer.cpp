#include <QtCore/qmath.h>
#include "SphereLayer.h"
#include <iostream>

// pi as a single precision float; used in computeSphere()
#define M_PI_F 3.14159265f

void SphereLayer::computeSphere (std::vector<float> positionOffset, VBOint& idx, const std::vector<float>& colour)
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
    DBG() << "Last vertex index for this sphere is: " << idx;
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

void SphereLayer::computeSphereGrid (void)
{
    // Spacing is set up here. Probably need to have a constructor
    // which passes in spacing, dims or even specific locations of
    // each sphere (neuron)
    float spacing = 0.1f;

    std::vector<float> po = {{ -(this->sidelen/2.0f*spacing), -(this->sidelen/2.0f*spacing), this->zposition }};
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

void SphereLayer::vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp) const
{
    vp.push_back (x);
    vp.push_back (y);
    vp.push_back (z);
}
