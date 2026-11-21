#ifndef MODELS_3D_H
#define MODELS_3D_H

#include <windows.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstring>
#include "sim_state.h"

namespace Models3D {

const float PI = 3.14159265f;

inline GLUquadric* getQuadric() {
    static GLUquadric* q = nullptr;
    if (!q) { q = gluNewQuadric(); gluQuadricNormals(q, GLU_SMOOTH); }
    return q;
}

inline void drawBox(float sx, float sy, float sz) {
    glPushMatrix(); glScalef(sx, sy, sz); glutSolidCube(1.0f); glPopMatrix();
}

inline void drawCylinderY(float baseR, float topR, float height, int slices = 16) {
    glPushMatrix(); glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(getQuadric(), baseR, topR, height, slices, 2);
    glPopMatrix();
}

// ============================================================
// PROCEDURAL TEXTURES - Generated in RAM at startup
// ============================================================
static GLuint texGrass = 0, texAsphalt = 0, texHospital = 0, texSnakeScale = 0, texBark = 0;
static bool texturesGenerated = false;

inline unsigned char clampByte(int v) { return (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v)); }

inline float hashf(float x, float y) {
    float v = std::sin(x * 127.1f + y * 311.7f) * 43758.5453f;
    return v - std::floor(v);
}

inline float noise2D(float x, float y) {
    float ix = std::floor(x), iy = std::floor(y);
    float fx = x - ix, fy = y - iy;
    float a = hashf(ix, iy), b = hashf(ix+1, iy);
    float c = hashf(ix, iy+1), d = hashf(ix+1, iy+1);
    float ux = fx*fx*(3.0f-2.0f*fx), uy = fy*fy*(3.0f-2.0f*fy);
    return a + (b-a)*ux + (c-a)*uy + (a-b-c+d)*ux*uy;
}

inline void generateTextures() {
    if (texturesGenerated) return;
    texturesGenerated = true;
    const int SZ = 128;
    unsigned char pixels[SZ * SZ * 3];

    // 1. Forest moss/grass & pebble dirt trail
    for (int y = 0; y < SZ; y++) for (int x = 0; x < SZ; x++) {
        float n = noise2D(x * 0.08f, y * 0.08f);
        float n2 = noise2D(x * 0.25f, y * 0.25f) * 0.4f;
        int idx = (y * SZ + x) * 3;
        pixels[idx+0] = clampByte((int)(40 + n * 60 + n2 * 30));
        pixels[idx+1] = clampByte((int)(80 + n * 90 + n2 * 40));
        pixels[idx+2] = clampByte((int)(30 + n * 40 + n2 * 20));
    }
    glGenTextures(1, &texGrass);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 2. Weathered highway asphalt
    for (int y = 0; y < SZ; y++) for (int x = 0; x < SZ; x++) {
        float n = noise2D(x * 0.15f, y * 0.15f);
        float grain = noise2D(x * 0.6f, y * 0.6f) * 0.3f;
        int base = (int)(38 + n * 20 + grain * 15);
        int idx = (y * SZ + x) * 3;
        pixels[idx+0] = clampByte(base);
        pixels[idx+1] = clampByte(base + 2);
        pixels[idx+2] = clampByte(base + 4);
    }
    glGenTextures(1, &texAsphalt);
    glBindTexture(GL_TEXTURE_2D, texAsphalt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 3. Hospital ceramic tile
    for (int y = 0; y < SZ; y++) for (int x = 0; x < SZ; x++) {
        int tx = x % 32, ty = y % 32;
        bool edge = (tx < 1 || ty < 1 || tx > 30 || ty > 30);
        float n = noise2D(x * 0.3f, y * 0.3f) * 8.0f;
        int idx = (y * SZ + x) * 3;
        if (edge) { pixels[idx+0]=180; pixels[idx+1]=185; pixels[idx+2]=190; }
        else {
            pixels[idx+0] = clampByte(210 + (int)n);
            pixels[idx+1] = clampByte(218 + (int)n);
            pixels[idx+2] = clampByte(225 + (int)n);
        }
    }
    glGenTextures(1, &texHospital);
    glBindTexture(GL_TEXTURE_2D, texHospital);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 4. Diamond/scale reptile pattern — brighter golden-tan/olive contrast
    for (int y = 0; y < SZ; y++) for (int x = 0; x < SZ; x++) {
        int dx = (x % 16) - 8, dy = (y % 16) - 8;
        int diamond = std::abs(dx) + std::abs(dy);
        float n = noise2D(x * 0.1f, y * 0.1f) * 20.0f;
        int idx = (y * SZ + x) * 3;
        if (diamond < 6) {
            pixels[idx+0] = clampByte(85 + (int)n);      // golden tan
            pixels[idx+1] = clampByte(72 + (int)n);
            pixels[idx+2] = clampByte(32 + (int)(n*0.4f));
        } else {
            pixels[idx+0] = clampByte(55 + (int)(n*0.8f)); // dark olive
            pixels[idx+1] = clampByte(52 + (int)(n*0.6f));
            pixels[idx+2] = clampByte(28 + (int)(n*0.3f));
        }
    }
    glGenTextures(1, &texSnakeScale);
    glBindTexture(GL_TEXTURE_2D, texSnakeScale);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 5. Tree bark
    for (int y = 0; y < SZ; y++) for (int x = 0; x < SZ; x++) {
        float n1 = noise2D(x * 0.05f, y * 0.3f);
        float n2 = noise2D(x * 0.2f, y * 0.08f) * 0.4f;
        int base = (int)(60 + n1 * 50 + n2 * 30);
        int idx = (y * SZ + x) * 3;
        pixels[idx+0] = clampByte(base + 15);
        pixels[idx+1] = clampByte(base);
        pixels[idx+2] = clampByte((int)(base * 0.5f));
    }
    glGenTextures(1, &texBark);
    glBindTexture(GL_TEXTURE_2D, texBark);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// ============================================================
// SHADOW DISC - Soft dark circle projected on ground
// ============================================================
inline void drawShadowDisc(float x, float z, float radius) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x, 0.02f, z);
    for (int i = 0; i <= 20; i++) {
        float a = i * 2.0f * PI / 20.0f;
        glVertex3f(x + std::cos(a) * radius, 0.02f, z + std::sin(a) * radius);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// ============================================================
// 3D SKY DOME
// ============================================================
inline void drawSkyDome(bool dusk) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    int stacks = 12, slices = 24;
    float R = 50.0f;
    for (int i = 0; i < stacks; i++) {
        float t0 = (float)i / stacks, t1 = (float)(i + 1) / stacks;
        float phi0 = t0 * PI * 0.5f, phi1 = t1 * PI * 0.5f;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta = j * 2.0f * PI / slices;
            for (int k = 1; k >= 0; k--) {
                float phi = (k == 0) ? phi0 : phi1;
                float t = (k == 0) ? t0 : t1;
                float nx = std::cos(phi) * std::cos(theta);
                float ny = std::sin(phi);
                float nz = std::cos(phi) * std::sin(theta);
                // Color gradient
                float r, g, b;
                if (dusk) {
                    r = 0.15f + t * 0.65f;
                    g = 0.10f + t * 0.25f + (1.0f - t) * 0.15f;
                    b = 0.25f + (1.0f - t) * 0.35f;
                } else {
                    r = 0.42f + t * 0.10f;
                    g = 0.65f + t * 0.10f;
                    b = 0.88f + t * 0.06f;
                    // Warm horizon
                    r += (1.0f - t) * 0.25f;
                    g += (1.0f - t) * 0.15f;
                }
                glColor3f(clampf(r,0,1), clampf(g,0,1), clampf(b,0,1));
                glVertex3f(nx * R, ny * R, nz * R);
            }
        }
        glEnd();
    }
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ============================================================
// PARTICLE RENDERING
// ============================================================
inline void renderParticles(const ParticleSystem& ps) {
    if (ps.count == 0) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < ps.count; i++) {
        const Particle& p = ps.particles[i];
        glColor4f(p.r, p.g, p.b, p.a * 0.8f);
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        glutSolidSphere(p.size, 6, 6);
        glPopMatrix();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// ============================================================
// 3D MEDICAL CROSS
// ============================================================
inline void drawMedicalCross3D(float scale) {
    glPushMatrix();
    glScalef(scale, scale, scale);
    glColor3f(0.88f, 0.12f, 0.15f);
    drawBox(1.4f, 0.42f, 0.28f);
    drawBox(0.42f, 1.4f, 0.28f);
    glPopMatrix();
}

// ============================================================
// CONTINUOUS LOFTED 3D COBRA MESH (GL_QUAD_STRIP)
// ============================================================
inline void drawSnake(const SimState& s, float x, float y, float z, float yawDeg, bool strikeAnim) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(yawDeg, 0.0f, 1.0f, 0.0f);

    // Strike lunge
    float lunge = 0.0f;
    if (strikeAnim && s.stageTime > 1.1f && s.stageTime < 1.9f) {
        float p = (s.stageTime - 1.1f) / 0.8f;
        lunge = std::sin(p * PI) * 1.15f;
    }
    glTranslatef(0.0f, 0.0f, lunge);

    // Enable snake scale texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texSnakeScale);

    // Build 24-segment spine curve
    const int SEGS = 24;
    float spineX[SEGS], spineY[SEGS], spineZ[SEGS], spineR[SEGS];
    for (int i = 0; i < SEGS; i++) {
        float t = (float)i / (SEGS - 1);
        spineZ[i] = -t * 2.8f;
        float wave = std::sin(s.simTime * 4.5f - i * 0.5f) * (0.12f + t * 0.25f);
        spineX[i] = wave;
        spineY[i] = 0.12f;

        // Raise front for cobra pose
        if (i < 6) {
            float lift = (6 - i) / 6.0f;
            spineY[i] += lift * 0.72f;
            spineX[i] *= 0.2f;
        }

        // Radius: thicker at hood, tapering to tail
        spineR[i] = 0.12f;
        if (i < 3) spineR[i] = 0.14f; // neck
        if (i >= 3 && i < 6 && s.isVenomous) spineR[i] = 0.08f; // just past hood
        spineR[i] *= (1.0f - t * 0.72f); // taper to tail
        if (spineR[i] < 0.015f) spineR[i] = 0.015f;
    }

    // Loft GL_QUAD_STRIP tube along spine
    const int RING = 10;
    glColor3f(0.52f, 0.42f, 0.22f);
    for (int i = 0; i < SEGS - 1; i++) {
        // Tangent for normals
        float tx = spineX[i+1] - spineX[i];
        float ty = spineY[i+1] - spineY[i];
        float tz = spineZ[i+1] - spineZ[i];
        float tlen = std::sqrt(tx*tx + ty*ty + tz*tz);
        if (tlen < 0.001f) tlen = 0.001f;
        tx /= tlen; ty /= tlen; tz /= tlen;

        // Build perpendicular basis vectors
        float ux, uy, uz;
        if (std::abs(ty) < 0.9f) { ux = 0; uy = 1; uz = 0; }
        else { ux = 1; uy = 0; uz = 0; }
        // Cross product: right = tangent x up
        float rx = ty*uz - tz*uy, ry = tz*ux - tx*uz, rz = tx*uy - ty*ux;
        float rlen = std::sqrt(rx*rx + ry*ry + rz*rz);
        if (rlen < 0.001f) rlen = 0.001f;
        rx /= rlen; ry /= rlen; rz /= rlen;
        // up = right x tangent
        ux = ry*tz - rz*ty; uy = rz*tx - rx*tz; uz = rx*ty - ry*tx;

        float texU0 = (float)i / (SEGS - 1);
        float texU1 = (float)(i + 1) / (SEGS - 1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= RING; j++) {
            float a = j * 2.0f * PI / RING;
            float ca = std::cos(a), sa = std::sin(a);
            float texV = (float)j / RING;

            for (int k = 0; k < 2; k++) {
                int si = i + k;
                float rad = spineR[si];
                // Hood flare on segments 2-4
                if (s.isVenomous && si >= 2 && si <= 4) {
                    float hoodFactor = 1.0f - std::abs(si - 3) * 0.3f;
                    float horiz = std::abs(ca);
                    rad += horiz * 0.22f * hoodFactor;
                }
                float nx = rx * ca + ux * sa;
                float ny = ry * ca + uy * sa;
                float nz = rz * ca + uz * sa;
                glNormal3f(nx, ny, nz);
                glTexCoord2f(k == 0 ? texU0 : texU1, texV);
                glVertex3f(spineX[si] + nx * rad, spineY[si] + ny * rad, spineZ[si] + nz * rad);
            }
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    // Hood spectacle markings (back of hood)
    if (s.isVenomous) {
        glColor3f(0.92f, 0.82f, 0.42f);
        glPushMatrix();
        glTranslatef(spineX[3] - 0.10f, spineY[3], spineZ[3] - 0.08f);
        glutSolidSphere(0.045f, 8, 8);
        glTranslatef(0.20f, 0.0f, 0.0f);
        glutSolidSphere(0.045f, 8, 8);
        glPopMatrix();
    }

    // HEAD - Angular cobra head
    float headWave = std::sin(s.simTime * 5.0f) * 0.03f;
    glPushMatrix();
    glTranslatef(spineX[0] + headWave, spineY[0] + 0.06f, spineZ[0] + 0.14f);

    // Upper skull
    glColor3f(0.22f, 0.18f, 0.11f);
    glPushMatrix();
    glScalef(1.2f, 0.65f, 1.6f);
    glutSolidSphere(0.12f, 12, 10);
    glPopMatrix();

    // Lower jaw (slight open during strike)
    float jawOpen = 0.0f;
    if (strikeAnim && s.stageTime > 1.0f && s.stageTime < 2.0f) {
        jawOpen = std::sin((s.stageTime - 1.0f) * PI) * 15.0f;
    }
    glPushMatrix();
    glTranslatef(0.0f, -0.06f, 0.04f);
    glRotatef(jawOpen, 1.0f, 0.0f, 0.0f);
    glColor3f(0.28f, 0.22f, 0.14f);
    glScalef(1.0f, 0.4f, 1.3f);
    glutSolidSphere(0.10f, 10, 8);
    glPopMatrix();

    // Glowing slit eyes
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 0.08f, 0.04f, 0.12f);
        // Eye glow
        glColor3f(0.98f, 0.80f, 0.08f);
        glutSolidSphere(0.028f, 8, 8);
        // Slit pupil
        glColor3f(0.05f, 0.05f, 0.02f);
        glPushMatrix();
        glScalef(0.3f, 1.0f, 0.3f);
        glutSolidSphere(0.025f, 6, 6);
        glPopMatrix();
        glPopMatrix();
    }

    // Twin curved white fangs (visible during strike/threat)
    if (strikeAnim || s.currentStage == STAGE_TITLE) {
        glColor3f(0.95f, 0.95f, 0.92f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.04f, -0.08f, 0.14f);
            glRotatef(10.0f, 0.0f, 0.0f, 1.0f * side);
            drawCylinderY(0.015f, 0.003f, 0.12f, 6);
            glPopMatrix();
        }
    }

    // Animated forked tongue
    float tonguePhase = std::sin(s.simTime * 8.0f);
    if (tonguePhase > 0.15f) {
        glColor3f(0.92f, 0.12f, 0.12f);
        glPushMatrix();
        glTranslatef(0.0f, -0.02f, 0.22f);
        float tongueLen = 0.12f + tonguePhase * 0.08f;
        drawBox(0.012f, 0.008f, tongueLen);
        // Fork
        glTranslatef(0.0f, 0.0f, tongueLen * 0.5f);
        glPushMatrix(); glRotatef(15.0f, 0.0f, 1.0f, 0.0f);
        drawBox(0.008f, 0.006f, 0.05f);
        glPopMatrix();
        glPushMatrix(); glRotatef(-15.0f, 0.0f, 1.0f, 0.0f);
        drawBox(0.008f, 0.006f, 0.05f);
        glPopMatrix();
        glPopMatrix();
    }

    glPopMatrix(); // head
    glPopMatrix(); // main transform
}

// ============================================================
// ARTICULATED 3D HUMAN (Tapered gluCylinder limbs + spherical joints)
// ============================================================
inline void drawHuman(bool seated, bool lying, bool hasSplint, bool showBiteGlow,
                      float simTime, float walkPhase, float shirtR, float shirtG, float shirtB) {
    glPushMatrix();

    if (lying) {
        glTranslatef(0.0f, 0.35f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    } else if (seated) {
        // Seated: pelvis on the ground at y~0.20
        glTranslatef(0.0f, 0.20f, 0.0f);
    }
    // Standing: no Y translate — shoes at y=0

    // bodyY = height from character origin to hip center
    // Standing: shoe(0.08) + shin(0.42) + knee(0.065) + thigh(0.42) = ~0.985 -> hip at ~0.42 from body origin
    // We need torso base at bodyY, with legs dangling below
    float bodyY = 0.0f;
    if (!seated && !lying) {
        // Standing: position body so shoe bottoms touch y=0
        // Total leg chain: hip(0.42) -> thigh(-0.44=knee) -> shin(-0.50=foot) -> shoe bottom
        // Hip at bodyY+0.42, so bodyY must place shoe bottom at y=0
        // bodyY + 0.42 - 0.44 - 0.50 - 0.04(shoe half) = 0  => bodyY = 0.56
        bodyY = 0.56f;
    }

    // Torso bob during walk
    float bob = 0.0f;
    if (!seated && !lying && std::abs(walkPhase) > 0.01f) {
        bob = std::abs(std::sin(walkPhase * 2.0f)) * 0.06f;
    }

    glPushMatrix();
    glTranslatef(0.0f, bodyY + bob, 0.0f);

    // TORSO - tapered cylinder
    glColor3f(shirtR, shirtG, shirtB);
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f);
    drawCylinderY(0.28f, 0.24f, 0.9f, 12);
    // Close top
    glPushMatrix(); glTranslatef(0.0f, 0.9f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(getQuadric(), 0.0, 0.24, 12, 1);
    glPopMatrix();
    glPopMatrix();

    // HEAD - connected cleanly to torso top
    glColor3f(0.88f, 0.70f, 0.56f);
    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.0f); // neck starts at torso top (0.45+0.9=1.35)
    // Neck
    drawCylinderY(0.10f, 0.09f, 0.20f, 8);
    glTranslatef(0.0f, 0.20f, 0.0f);
    glutSolidSphere(0.22f, 14, 12);
    // Hair
    glColor3f(0.14f, 0.11f, 0.09f);
    glPushMatrix();
    glTranslatef(0.0f, 0.08f, -0.02f);
    glScalef(1.02f, 0.78f, 1.02f);
    glutSolidSphere(0.21f, 12, 10);
    glPopMatrix();
    glPopMatrix();

    // ARMS with 2 joints (shoulder sphere + elbow sphere)
    // Shoulders attach at torso top: y = 0.45 + 0.9 - 0.05 = 1.30
    float armSwing = (!seated && !lying) ? std::sin(walkPhase) * 28.0f : 0.0f;
    float elbowBend = (!seated && !lying) ? (25.0f + std::abs(std::sin(walkPhase)) * 20.0f) : (seated ? 35.0f : 0.0f);

    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        // Shoulder joint — flush with upper torso
        glTranslatef(side * 0.30f, 1.28f, 0.0f);
        glColor3f(shirtR * 0.9f, shirtG * 0.9f, shirtB * 0.9f);
        glutSolidSphere(0.08f, 8, 8); // shoulder ball

        float swing = side * armSwing;
        glRotatef(swing, 1.0f, 0.0f, 0.0f);

        // Upper arm
        glColor3f(shirtR, shirtG, shirtB);
        glPushMatrix();
        glTranslatef(0.0f, -0.22f, 0.0f);
        drawCylinderY(0.07f, 0.06f, 0.35f, 8);
        glPopMatrix();

        // Elbow joint
        glTranslatef(0.0f, -0.38f, 0.0f);
        glColor3f(0.86f, 0.68f, 0.54f);
        glutSolidSphere(0.06f, 8, 8);
        glRotatef(-elbowBend, 1.0f, 0.0f, 0.0f);

        // Forearm
        glColor3f(0.86f, 0.68f, 0.54f);
        glPushMatrix();
        glTranslatef(0.0f, -0.2f, 0.0f);
        drawCylinderY(0.055f, 0.045f, 0.3f, 8);
        glPopMatrix();

        // Hand
        glTranslatef(0.0f, -0.35f, 0.0f);
        glutSolidSphere(0.055f, 8, 8);
        glPopMatrix();
    }

    // LEGS with hip + knee joints
    float legSwing = (!seated && !lying) ? std::sin(walkPhase) * 32.0f : 0.0f;
    float kneeBend = (!seated && !lying) ? std::max(0.0f, std::sin(walkPhase) * 35.0f) : 0.0f;

    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        // Hip joint — directly at torso bottom
        glTranslatef(side * 0.14f, 0.42f, 0.0f);
        glColor3f(0.22f, 0.25f, 0.29f);
        glutSolidSphere(0.08f, 8, 8); // hip ball

        if (seated) {
            glRotatef(-86.0f, 1.0f, 0.0f, 0.0f);
        } else if (!lying) {
            glRotatef(-side * legSwing, 1.0f, 0.0f, 0.0f);
        }

        // Upper leg (thigh) — starts immediately below hip
        glPushMatrix();
        glTranslatef(0.0f, -0.08f, 0.0f);
        drawCylinderY(0.09f, 0.07f, 0.36f, 10);
        glPopMatrix();

        // Knee joint
        glTranslatef(0.0f, -0.44f, 0.0f);
        glColor3f(0.22f, 0.25f, 0.29f);
        glutSolidSphere(0.065f, 8, 8);

        if (!seated && !lying) {
            float kb = (side == 1) ? kneeBend : std::max(0.0f, -std::sin(walkPhase) * 35.0f);
            glRotatef(kb, 1.0f, 0.0f, 0.0f);
        }

        // Lower leg (shin) — starts immediately below knee
        glPushMatrix();
        glTranslatef(0.0f, -0.065f, 0.0f);
        drawCylinderY(0.065f, 0.05f, 0.38f, 10);
        glPopMatrix();

        // Right leg: splint, bandage, fang marks
        if (side == 1) {
            if (hasSplint) {
                glColor3f(0.68f, 0.48f, 0.24f);
                glPushMatrix();
                glTranslatef(0.12f, -0.30f, 0.0f);
                drawBox(0.04f, 0.55f, 0.10f);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(-0.12f, -0.30f, 0.0f);
                drawBox(0.04f, 0.55f, 0.10f);
                glPopMatrix();
                // White wrap bands
                glColor3f(0.95f, 0.96f, 0.98f);
                for (float by = -0.46f; by <= -0.1f; by += 0.16f) {
                    glPushMatrix();
                    glTranslatef(0.0f, by, 0.0f);
                    drawCylinderY(0.085f, 0.085f, 0.06f, 10);
                    glPopMatrix();
                }
            }
            if (showBiteGlow) {
                float pulse = 0.55f + 0.45f * std::sin(simTime * 7.0f);
                glColor3f(0.95f, 0.12f * pulse, 0.12f * pulse);
                glPushMatrix();
                glTranslatef(-0.03f, -0.45f, 0.07f);
                glutSolidSphere(0.030f, 8, 8);
                glTranslatef(0.06f, 0.0f, 0.0f);
                glutSolidSphere(0.030f, 8, 8);
                glPopMatrix();
            }
        }

        // Foot/shoe — directly at bottom of lower leg (zero gap)
        glTranslatef(0.0f, -0.50f, 0.06f);
        glColor3f(0.12f, 0.12f, 0.13f);
        glPushMatrix();
        glScalef(1.0f, 0.55f, 1.5f);
        glutSolidSphere(0.085f, 8, 8);
        glPopMatrix();

        glPopMatrix();
    }

    glPopMatrix(); // body Y offset
    glPopMatrix(); // main
}

// ============================================================
// ENVIRONMENT PROPS
// ============================================================
inline void drawPineTree(float x, float z, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glScalef(scale, scale, scale);

    // Textured trunk
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBark);
    glColor3f(0.55f, 0.38f, 0.22f);
    GLUquadric* tq = gluNewQuadric();
    gluQuadricTexture(tq, GL_TRUE);
    gluQuadricNormals(tq, GLU_SMOOTH);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(tq, 0.22f, 0.16f, 1.8f, 10, 2);
    glPopMatrix();
    gluDeleteQuadric(tq);
    glDisable(GL_TEXTURE_2D);

    // 3 foliage cones
    float colors[][3] = {{0.11f,0.36f,0.14f},{0.14f,0.43f,0.17f},{0.18f,0.50f,0.21f}};
    float heights[] = {1.4f, 2.4f, 3.3f};
    float radii[] = {1.25f, 0.95f, 0.62f};
    float coneH[] = {2.0f, 1.7f, 1.3f};
    for (int i = 0; i < 3; i++) {
        glColor3f(colors[i][0], colors[i][1], colors[i][2]);
        glPushMatrix();
        glTranslatef(0.0f, heights[i], 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(radii[i], coneH[i], 12, 3);
        glPopMatrix();
    }
    glPopMatrix();
    // Shadow
    drawShadowDisc(x, z, 1.2f * scale);
}

inline void drawRock(float x, float z, float sx, float sy, float sz) {
    glPushMatrix();
    glTranslatef(x, sy * 0.4f, z);
    glScalef(sx, sy, sz);
    glColor3f(0.52f, 0.50f, 0.46f);
    glutSolidSphere(1.0f, 8, 6);
    glPopMatrix();
}

// ============================================================
// TEXTURED GROUND QUAD
// ============================================================
inline void drawTexturedGround(GLuint tex, float x0, float z0, float x1, float z1, float y,
                               float r, float g, float b, float texScale = 4.0f) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y, z0);
    glTexCoord2f(0.0f, texScale); glVertex3f(x0, y, z1);
    glTexCoord2f(texScale, texScale); glVertex3f(x1, y, z1);
    glTexCoord2f(texScale, 0.0f); glVertex3f(x1, y, z0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// ============================================================
// FOREST ENVIRONMENT
// ============================================================
inline void drawForestEnvironment() {
    // Textured grass
    drawTexturedGround(texGrass, -35.0f, -35.0f, 35.0f, 35.0f, 0.0f, 0.90f, 0.95f, 0.90f, 8.0f);

    // Dirt trail
    glColor3f(0.52f, 0.40f, 0.26f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-2.6f, 0.015f, -30.0f);
    glVertex3f(-2.6f, 0.015f, 30.0f);
    glVertex3f(2.6f, 0.015f, 30.0f);
    glVertex3f(2.6f, 0.015f, -30.0f);
    glEnd();

    // Trees
    drawPineTree(-6.5f, -7.0f, 1.2f);
    drawPineTree(-8.0f, 1.5f, 1.0f);
    drawPineTree(-5.8f, 7.5f, 1.3f);
    drawPineTree(6.2f, -8.5f, 1.15f);
    drawPineTree(7.5f, -1.0f, 1.35f);
    drawPineTree(6.0f, 6.5f, 1.05f);
    drawPineTree(-2.5f, -12.0f, 1.4f);
    drawPineTree(3.2f, -13.0f, 1.25f);
    drawPineTree(-4.5f, -16.0f, 1.1f);
    drawPineTree(5.5f, -15.0f, 1.3f);

    // Rocks
    drawRock(-3.3f, 2.2f, 0.42f, 0.28f, 0.35f);
    drawRock(3.6f, -2.8f, 0.55f, 0.32f, 0.48f);
    drawRock(2.9f, 3.5f, 0.30f, 0.22f, 0.28f);
}

// Extraction zone rendering — only called during Stage 3
inline void drawExtractionZone(const SimState& s) {
    float pulse = 0.55f + 0.45f * std::sin(s.simTime * 4.0f);

    // Pulsing green glowing pad
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.95f, 0.35f, 0.22f + 0.18f * pulse);
    glBegin(GL_QUADS);
    glVertex3f(-3.0f, 0.04f, -14.0f);
    glVertex3f(-3.0f, 0.04f, -11.0f);
    glVertex3f(3.0f, 0.04f, -11.0f);
    glVertex3f(3.0f, 0.04f, -14.0f);
    glEnd();

    // Bright green border
    glColor4f(0.2f, 1.0f, 0.4f, 0.7f * pulse);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-3.0f, 0.05f, -14.0f);
    glVertex3f(-3.0f, 0.05f, -11.0f);
    glVertex3f(3.0f, 0.05f, -11.0f);
    glVertex3f(3.0f, 0.05f, -14.0f);
    glEnd();
    glLineWidth(1.0f);

    // Vertical beacon pillars at corners
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.1f, 0.95f, 0.4f, 0.12f * pulse);
    for (int cx = -1; cx <= 1; cx += 2) {
        for (int cz = -1; cz <= 1; cz += 2) {
            float bx = cx * 2.8f;
            float bz = -12.5f + cz * 1.3f;
            glBegin(GL_QUADS);
            glVertex3f(bx - 0.08f, 0.05f, bz - 0.08f);
            glVertex3f(bx + 0.08f, 0.05f, bz - 0.08f);
            glVertex3f(bx + 0.08f, 3.5f, bz + 0.08f);
            glVertex3f(bx - 0.08f, 3.5f, bz + 0.08f);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // "EXTRACTION" text marker at zone center (green glow disc)
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.15f, 1.0f, 0.45f, 0.3f * pulse);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.045f, -12.5f);
    for (int i = 0; i <= 20; i++) {
        float a = i * 2.0f * PI / 20.0f;
        glVertex3f(std::cos(a) * 1.5f, 0.045f, -12.5f + std::sin(a) * 1.5f);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // Static ambulance model at extraction zone
    glPushMatrix();
    glTranslatef(3.5f, 0.0f, -12.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.6f, 0.6f, 0.6f);
    // Simple mini ambulance
    glColor3f(0.94f, 0.95f, 0.97f);
    glPushMatrix(); glTranslatef(0.0f, 0.9f, 0.0f); drawBox(1.4f, 1.2f, 2.5f); glPopMatrix();
    glColor3f(0.86f, 0.14f, 0.16f);
    glPushMatrix(); glTranslatef(0.0f, 0.65f, 0.0f); drawBox(1.44f, 0.16f, 2.52f); glPopMatrix();
    // Wheels
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int sx = -1; sx <= 1; sx += 2) for (int sz = -1; sz <= 1; sz += 2) {
        glPushMatrix(); glTranslatef(sx * 0.72f, 0.15f, sz * 0.85f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidTorus(0.1f, 0.2f, 8, 12);
        glPopMatrix();
    }
    glPopMatrix();
}

// ============================================================
// STRETCHER
// ============================================================
inline void drawStretcher(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(0.85f, 0.90f, 0.94f);
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); drawBox(1.15f, 0.14f, 2.55f); glPopMatrix();
    // Red straps
    glColor3f(0.85f, 0.18f, 0.18f);
    glPushMatrix(); glTranslatef(0.0f, 0.63f, -0.35f); drawBox(1.18f, 0.04f, 0.12f);
    glTranslatef(0.0f, 0.0f, 0.85f); drawBox(1.18f, 0.04f, 0.12f); glPopMatrix();
    // Frame
    glColor3f(0.65f, 0.68f, 0.72f);
    for (int sx = -1; sx <= 1; sx += 2) {
        glPushMatrix(); glTranslatef(sx * 0.56f, 0.52f, 0.0f); drawBox(0.06f, 0.06f, 2.95f); glPopMatrix();
        for (int sz = -1; sz <= 1; sz += 2) {
            glPushMatrix(); glTranslatef(sx * 0.48f, 0.25f, sz * 1.05f); drawBox(0.06f, 0.50f, 0.06f); glPopMatrix();
        }
    }
    glPopMatrix();
    drawShadowDisc(x, z, 1.0f);
}

// ============================================================
// 3D AMBULANCE (Enhanced with beveled hood, translucent windshield, wheel wells, headlight cones)
// ============================================================
inline void drawAmbulance(const SimState& s, float posX) {
    float bounce = std::sin(s.simTime * (s.sirenActive ? 22.0f : 12.0f)) * 0.03f;
    glPushMatrix();
    glTranslatef(posX, 0.48f + bounce, 0.0f);

    // Main cabin (white)
    glColor3f(0.94f, 0.95f, 0.97f);
    glPushMatrix(); glTranslatef(0.0f, 1.15f, 0.55f); drawBox(2.25f, 1.85f, 3.65f); glPopMatrix();

    // Red stripe
    glColor3f(0.86f, 0.14f, 0.16f);
    glPushMatrix(); glTranslatef(0.0f, 0.95f, 0.55f); drawBox(2.29f, 0.24f, 3.68f); glPopMatrix();

    // Front hood (beveled)
    glColor3f(0.92f, 0.94f, 0.96f);
    glPushMatrix(); glTranslatef(0.0f, 0.65f, -1.95f); drawBox(2.15f, 0.85f, 1.45f); glPopMatrix();

    // Sloped windshield (translucent)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.16f, 0.28f, 0.42f, 0.7f);
    glPushMatrix(); glTranslatef(0.0f, 1.55f, -1.32f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f); drawBox(1.95f, 0.72f, 0.08f); glPopMatrix();
    glDisable(GL_BLEND);

    // Side windows (translucent)
    glEnable(GL_BLEND);
    glColor4f(0.18f, 0.30f, 0.44f, 0.6f);
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 1.14f, 1.5f, 0.0f);
        drawBox(0.06f, 0.5f, 1.8f);
        glPopMatrix();
    }
    glDisable(GL_BLEND);

    // Headlights
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 0.75f, 0.65f, -2.69f);
        // Yellow headlight box
        glColor3f(1.0f, 0.96f, 0.72f);
        drawBox(0.38f, 0.22f, 0.08f);

        // Flat ground-projected fading light beam
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glBegin(GL_QUADS);
        glColor4f(1.0f, 0.96f, 0.72f, 0.30f);
        glVertex3f(-0.30f, -1.10f, -0.1f);
        glVertex3f( 0.30f, -1.10f, -0.1f);
        glColor4f(1.0f, 0.96f, 0.72f, 0.0f);
        glVertex3f( 0.95f, -1.10f, -6.0f);
        glVertex3f(-0.95f, -1.10f, -6.0f);
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Medical crosses on sides
    glPushMatrix(); glTranslatef(1.14f, 1.42f, 0.6f); glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawMedicalCross3D(0.55f); glPopMatrix();
    glPushMatrix(); glTranslatef(-1.14f, 1.42f, 0.6f); glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    drawMedicalCross3D(0.55f); glPopMatrix();

    // Flashing beacons with dynamic GL_LIGHT1
    bool redFlash = std::sin(s.simTime * (s.sirenActive ? 18.0f : 6.0f)) > 0.0f;
    glPushMatrix(); glTranslatef(-0.55f, 2.18f, -0.95f);
    glColor3f(redFlash ? 1.0f : 0.35f, redFlash ? 0.12f : 0.06f, redFlash ? 0.12f : 0.06f);
    glutSolidSphere(0.20f, 12, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(0.55f, 2.18f, -0.95f);
    glColor3f(!redFlash ? 0.12f : 0.06f, !redFlash ? 0.45f : 0.12f, !redFlash ? 1.0f : 0.35f);
    glutSolidSphere(0.20f, 12, 10); glPopMatrix();

    // 4 wheels with spinning hubcaps
    float wheelSpin = -s.simTime * (s.sirenActive ? 680.0f : 360.0f);
    float wx[2] = {-1.12f, 1.12f};
    float wz[2] = {-1.75f, 1.45f};
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        glPushMatrix();
        glTranslatef(wx[i], 0.0f, wz[j]);
        glRotatef(wheelSpin, 1.0f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glColor3f(0.10f, 0.10f, 0.11f);
        glutSolidTorus(0.16f, 0.32f, 10, 16);
        glColor3f(0.75f, 0.77f, 0.80f);
        drawBox(0.45f, 0.09f, 0.12f);
        drawBox(0.12f, 0.09f, 0.45f);
        glPopMatrix();
    }

    glPopMatrix();
    drawShadowDisc(posX, 0.0f, 2.2f);
}

// ============================================================
// OBSTACLE CARS (Stage 4)
// ============================================================
inline void drawObstacleCar(float x, float z, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, 0.4f, z);
    // Body
    glColor3f(r, g, b);
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); drawBox(1.6f, 0.8f, 3.0f); glPopMatrix();
    // Roof
    glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
    glPushMatrix(); glTranslatef(0.0f, 0.95f, 0.1f); drawBox(1.4f, 0.5f, 1.6f); glPopMatrix();
    // Windshield
    glColor3f(0.2f, 0.3f, 0.5f);
    glPushMatrix(); glTranslatef(0.0f, 0.85f, -0.75f);
    glRotatef(-25.0f, 1.0f, 0.0f, 0.0f); drawBox(1.3f, 0.5f, 0.06f); glPopMatrix();
    // Wheels
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int sx = -1; sx <= 1; sx += 2) for (int sz = -1; sz <= 1; sz += 2) {
        glPushMatrix(); glTranslatef(sx * 0.75f, -0.1f, sz * 0.9f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidTorus(0.1f, 0.2f, 8, 12);
        glPopMatrix();
    }
    // Taillights
    glColor3f(1.0f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(-0.6f, 0.35f, 1.52f); drawBox(0.2f, 0.12f, 0.04f);
    glTranslatef(1.2f, 0.0f, 0.0f); drawBox(0.2f, 0.12f, 0.04f); glPopMatrix();
    glPopMatrix();
}

// ============================================================
// AMBULANCE HIGHWAY SCENE (Stage 4)
// ============================================================
inline void drawAmbulanceScene(const SimState& s) {
    drawSkyDome(true); // dusk sky

    // Fog for depth
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.15f, 0.12f, 0.22f, 1.0f};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 20.0f);
    glFogf(GL_FOG_END, 55.0f);

    // Textured asphalt highway
    drawTexturedGround(texAsphalt, -4.5f, -45.0f, 4.5f, 45.0f, 0.0f, 0.90f, 0.90f, 0.90f, 6.0f);
    // Grass shoulders
    drawTexturedGround(texGrass, -35.0f, -45.0f, -4.5f, 45.0f, -0.02f, 0.75f, 0.80f, 0.75f, 8.0f);
    drawTexturedGround(texGrass, 4.5f, -45.0f, 35.0f, 45.0f, -0.02f, 0.75f, 0.80f, 0.75f, 8.0f);

    // Lane markings (3 lanes: -2.5, 0, 2.5)
    glColor3f(0.95f, 0.85f, 0.22f);
    for (float lx = -1.25f; lx <= 1.25f; lx += 2.5f) {
        for (float z = -40.0f; z <= 40.0f; z += 5.0f) {
            float mz = z + std::fmod(s.roadScrollOffset, 5.0f);
            glPushMatrix(); glTranslatef(lx, 0.02f, mz); drawBox(0.14f, 0.02f, 2.0f); glPopMatrix();
        }
    }

    // Scrolling roadside trees
    for (float z = -40.0f; z <= 40.0f; z += 10.0f) {
        float tz = z + std::fmod(s.roadScrollOffset, 10.0f);
        drawPineTree(-7.5f, tz, 1.15f);
        drawPineTree(7.5f, tz - 4.0f, 1.15f);
    }

    // Obstacle cars
    for (int i = 0; i < SimState::MAX_OBSTACLES; i++) {
        if (!s.obstacles[i].active) continue;
        drawObstacleCar(s.obstacles[i].x, s.obstacles[i].z,
                       s.obstacles[i].r, s.obstacles[i].g, s.obstacles[i].b);
    }

    // The ambulance
    drawAmbulance(s, s.ambulanceActualX);

    glDisable(GL_FOG);
}

// ============================================================
// HOSPITAL ER SCENE (Stage 5)
// ============================================================
inline void drawHospitalERScene(const SimState& s) {
    // Textured tiled floor — wider
    drawTexturedGround(texHospital, -12.0f, -12.0f, 12.0f, 12.0f, 0.0f, 0.95f, 0.95f, 0.95f, 8.0f);

    // Walls — expanded
    glColor3f(0.88f, 0.93f, 0.95f);
    glPushMatrix(); glTranslatef(0.0f, 3.0f, -5.0f); drawBox(24.0f, 6.0f, 0.25f); glPopMatrix();
    glPushMatrix(); glTranslatef(-8.5f, 3.0f, 0.0f); drawBox(0.25f, 6.0f, 14.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(8.5f, 3.0f, 0.0f); drawBox(0.25f, 6.0f, 14.0f); glPopMatrix();

    // Wall cross
    glPushMatrix(); glTranslatef(-3.2f, 3.8f, -4.82f); drawMedicalCross3D(0.75f); glPopMatrix();

    // ECG Wall Monitor
    glPushMatrix();
    glTranslatef(2.6f, 3.2f, -4.75f);
    glColor3f(0.14f, 0.16f, 0.19f);
    drawBox(2.4f, 1.5f, 0.22f);
    glColor3f(0.03f, 0.11f, 0.08f);
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.12f); drawBox(2.15f, 1.25f, 0.04f); glPopMatrix();

    // Live ECG trace
    glDisable(GL_LIGHTING);
    if (s.heartRate > 130.0f) glColor3f(1.0f, 0.28f, 0.28f);
    else if (s.heartRate > 105.0f) glColor3f(1.0f, 0.78f, 0.22f);
    else glColor3f(0.15f, 0.98f, 0.45f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 40; i++) {
        float u = (float)i / 40.0f;
        float ex = -0.98f + u * 1.96f;
        float phase = std::fmod(u * 3.0f + s.ecgPhase * 0.4f, 1.0f);
        float ey = 0.0f;
        if (phase > 0.40f && phase < 0.45f) ey = 0.12f;
        else if (phase >= 0.45f && phase < 0.50f) ey = -0.14f;
        else if (phase >= 0.50f && phase < 0.55f) ey = 0.44f;
        else if (phase >= 0.55f && phase < 0.60f) ey = -0.22f;
        else if (phase >= 0.60f && phase < 0.68f) ey = 0.10f;
        glVertex3f(ex, ey, 0.16f);
    }
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Hospital Bed
    glPushMatrix();
    glColor3f(0.95f, 0.97f, 0.99f);
    glPushMatrix(); glTranslatef(0.0f, 1.05f, 0.0f); drawBox(1.85f, 0.35f, 3.6f); glPopMatrix();
    glColor3f(0.28f, 0.62f, 0.78f);
    glPushMatrix(); glTranslatef(0.0f, 1.12f, 0.65f); drawBox(1.88f, 0.28f, 2.0f); glPopMatrix();
    glColor3f(0.55f, 0.60f, 0.66f);
    glPushMatrix(); glTranslatef(0.0f, 1.45f, -1.78f); drawBox(1.92f, 1.35f, 0.12f); glPopMatrix();
    for (int sx = -1; sx <= 1; sx += 2) for (int sz = -1; sz <= 1; sz += 2) {
        glPushMatrix(); glTranslatef(sx * 0.85f, 0.45f, sz * 1.65f);
        drawBox(0.10f, 0.90f, 0.10f); glPopMatrix();
    }
    glPopMatrix();
    drawShadowDisc(0.0f, 0.0f, 1.5f);

    // Patient on bed — raised slightly, shifted forward
    glPushMatrix();
    glTranslatef(0.0f, 1.33f, -0.05f);
    drawHuman(false, true, true, s.venomLevel > 10.0f, s.simTime, 0.0f, 0.65f, 0.78f, 0.85f);
    glPopMatrix();

    // White pillow at head of bed
    glColor3f(0.96f, 0.97f, 0.99f);
    glPushMatrix(); glTranslatef(0.0f, 1.28f, -1.55f);
    glScalef(1.0f, 0.4f, 1.0f);
    glutSolidSphere(0.28f, 10, 8);
    glPopMatrix();

    // Doctor standing closer to bedside with subtle breathing
    float breathOsc = std::sin(s.simTime * 2.5f) * 0.015f;
    glPushMatrix();
    glTranslatef(1.2f, breathOsc, 0.3f);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); // face the bed
    drawHuman(false, false, false, false, s.simTime, 0.0f, 0.90f, 0.92f, 0.95f);
    glPopMatrix();
    drawShadowDisc(1.2f, 0.3f, 0.5f);
    // Stethoscope around doctor's neck (dark torus)
    glColor3f(0.15f, 0.15f, 0.18f);
    glPushMatrix();
    glTranslatef(1.2f, 1.85f + breathOsc, 0.3f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.018f, 0.10f, 8, 16);
    glPopMatrix();

    // IV Stand
    glPushMatrix();
    glTranslatef(1.55f, 0.0f, -0.75f);
    glColor3f(0.68f, 0.72f, 0.76f);
    drawBox(0.75f, 0.08f, 0.75f);
    drawCylinderY(0.04f, 0.04f, 3.55f, 10);
    glPushMatrix(); glTranslatef(-0.28f, 3.45f, 0.0f); drawBox(0.58f, 0.05f, 0.05f); glPopMatrix();

    // IV Bag
    glPushMatrix();
    glTranslatef(-0.48f, 3.05f, 0.0f);
    glColor3f(0.86f, 0.93f, 0.97f);
    drawBox(0.28f, 0.56f, 0.12f);
    float fluidH = 0.48f * clampf(s.ivFluidLevel, 0.02f, 1.0f);
    if (s.asvAdministered) glColor3f(0.12f, 0.85f, 0.65f);
    else glColor3f(0.45f, 0.75f, 0.88f);
    glPushMatrix(); glTranslatef(0.0f, -0.24f + fluidH * 0.5f, 0.01f);
    drawBox(0.24f, fluidH, 0.13f); glPopMatrix();
    glPopMatrix();

    // Animated drip droplet
    if (s.asvAdministered && s.ivFluidLevel > 0.05f) {
        float dropY = 2.65f - std::fmod(s.simTime * 2.2f, 1.25f);
        glColor3f(0.20f, 0.95f, 0.75f);
        glPushMatrix(); glTranslatef(-0.48f, dropY, 0.0f);
        glutSolidSphere(0.045f, 8, 8); glPopMatrix();
    }

    // IV tube to patient's arm — extended path
    if (s.asvAdministered) {
        glDisable(GL_LIGHTING);
        glColor3f(0.4f, 0.9f, 0.7f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-0.48f + 1.55f, 2.5f, -0.75f);  // from bag
        glVertex3f(0.9f, 2.0f, -0.5f);              // curve down
        glVertex3f(0.5f, 1.6f, -0.3f);              // approach bed
        glVertex3f(0.3f, 1.45f, -0.1f);             // to patient's arm
        glVertex3f(0.15f, 1.40f, 0.0f);             // at arm
        glEnd();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    glPopMatrix(); // IV stand
}

// ============================================================
// STAGE 0 / 6 SHOWCASE
// ============================================================
inline void drawShowcaseScene(const SimState& s) {
    // Subtle receding 3D floor grid beneath pedestal
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    for (int i = -12; i <= 12; i++) {
        float fi = (float)i;
        float fade = 1.0f - std::abs(fi) / 14.0f;
        glColor4f(0.15f, 0.35f, 0.55f, 0.18f * fade);
        glBegin(GL_LINES);
        glVertex3f(fi, 0.01f, -12.0f); glVertex3f(fi, 0.01f, 12.0f);
        glVertex3f(-12.0f, 0.01f, fi); glVertex3f(12.0f, 0.01f, fi);
        glEnd();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // Warm overhead spotlight (GL_LIGHT1) for title screen
    glEnable(GL_LIGHT1);
    GLfloat spotPos[] = { 0.0f, 6.0f, 2.0f, 1.0f };
    GLfloat spotDif[] = { 1.0f, 0.90f, 0.65f, 1.0f };
    GLfloat spotAmb[] = { 0.12f, 0.10f, 0.06f, 1.0f };
    GLfloat spotDir[] = { 0.0f, -1.0f, -0.3f };
    glLightfv(GL_LIGHT1, GL_POSITION, spotPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, spotDif);
    glLightfv(GL_LIGHT1, GL_AMBIENT, spotAmb);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spotDif);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 35.0f);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 12.0f);

    glPushMatrix();
    glRotatef(s.simTime * 18.0f, 0.0f, 1.0f, 0.0f);

    // Pedestal cylinder
    glColor3f(0.22f, 0.26f, 0.32f);
    drawCylinderY(2.6f, 2.4f, 0.35f, 28);
    // Solid cap disk on top
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glColor3f(0.30f, 0.34f, 0.40f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(getQuadric(), 0.0, 2.4, 28, 1);
    glPopMatrix();
    // Glowing cyan rim ring on top edge
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float rimPulse = 0.65f + 0.35f * std::sin(s.simTime * 3.5f);
    glColor4f(0.1f, 0.85f, 0.95f, 0.55f * rimPulse);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 36; i++) {
        float a = i * 2.0f * PI / 36.0f;
        glVertex3f(std::cos(a) * 2.42f, 0.36f, std::sin(a) * 2.42f);
    }
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // Medical cross — higher and behind cobra, inside a rotating cyan halo ring
    glPushMatrix();
    glTranslatef(0.0f, 2.8f, -1.2f);
    drawMedicalCross3D(1.1f);
    // Rotating cyan halo ring around the cross
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.0f, 0.90f, 0.95f, 0.35f * rimPulse);
    glLineWidth(2.0f);
    glPushMatrix();
    glRotatef(s.simTime * 55.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float a = i * 2.0f * PI / 32.0f;
        glVertex3f(std::cos(a) * 1.1f, std::sin(a) * 1.1f, 0.0f);
    }
    glEnd();
    glPopMatrix();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Snake with brighter golden-tan contrast
    drawSnake(s, 0.0f, 0.35f, 0.85f, 15.0f, false);
    glPopMatrix();
    drawShadowDisc(0.0f, 0.0f, 2.5f);

    // Title ambient dust/glow particles
    // (emitted in sim_state update when STAGE_TITLE)
}

// ============================================================
// LIGHTING CONFIGURATION
// ============================================================
inline void configureLighting(const SimState& s) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);

    GLfloat light0Pos[] = { 8.0f, 14.0f, 10.0f, 1.0f };
    GLfloat light0Amb[] = { 0.28f, 0.28f, 0.30f, 1.0f };
    GLfloat light0Dif[] = { 0.92f, 0.92f, 0.90f, 1.0f };
    GLfloat light0Spc[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spc);

    if (s.currentStage == STAGE_TRANSPORT) {
        glEnable(GL_LIGHT1);
        bool redFlash = std::sin(s.simTime * (s.sirenActive ? 18.0f : 6.0f)) > 0.0f;
        GLfloat beaconPos[] = { s.ambulanceActualX, 3.2f, -0.9f, 1.0f };
        GLfloat beaconCol[] = { redFlash ? 0.95f : 0.10f, 0.12f, redFlash ? 0.10f : 0.95f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, beaconPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, beaconCol);
        // Reset spotlight params (not a spotlight for transport)
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0f);
    } else if (s.currentStage != STAGE_TITLE && s.currentStage != STAGE_DEBRIEF) {
        // Title/Debrief set their own GL_LIGHT1 in drawShowcaseScene
        glDisable(GL_LIGHT1);
    }
}

// ============================================================
// MASTER 3D SCENE RENDERER
// ============================================================
inline void renderScene3D(SimState& s) {
    generateTextures();

    // Sky/room clear color
    if (s.currentStage == STAGE_TITLE || s.currentStage == STAGE_DEBRIEF)
        glClearColor(0.08f, 0.11f, 0.17f, 1.0f);
    else if (s.currentStage == STAGE_HOSPITAL)
        glClearColor(0.75f, 0.83f, 0.88f, 1.0f);
    else if (s.currentStage == STAGE_TRANSPORT)
        glClearColor(0.15f, 0.12f, 0.22f, 1.0f);
    else
        glClearColor(0.52f, 0.76f, 0.94f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configureLighting(s);

    // Sky dome for outdoor stages
    if (s.currentStage == STAGE_RECOGNITION || s.currentStage == STAGE_FIRST_AID ||
        s.currentStage == STAGE_HANDLING) {
        drawSkyDome(false);
    }

    // Fog for forest
    if (s.currentStage >= STAGE_RECOGNITION && s.currentStage <= STAGE_HANDLING) {
        glEnable(GL_FOG);
        GLfloat fogCol[] = {0.52f, 0.76f, 0.94f, 1.0f};
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, fogCol);
        glFogf(GL_FOG_START, 25.0f);
        glFogf(GL_FOG_END, 50.0f);
    }

    switch (s.currentStage) {
        case STAGE_TITLE:
        case STAGE_DEBRIEF:
            drawShowcaseScene(s);
            break;

        case STAGE_RECOGNITION: {
            drawForestEnvironment();
            // Player hiker
            glPushMatrix();
            glTranslatef(s.playerX, 0.0f, s.playerZ);
            glRotatef(s.playerAngle, 0.0f, 1.0f, 0.0f);
            drawHuman(false, false, false, s.strikeComplete, s.simTime,
                     s.playerMoving ? s.playerWalkPhase : 0.0f, 0.18f, 0.46f, 0.72f);
            glPopMatrix();
            drawShadowDisc(s.playerX, s.playerZ, 0.5f);
            // Snake — uses dynamic position from sim_state
            drawSnake(s, s.snakeX, 0.0f, s.snakeZ, s.snakeTargetYaw, !s.strikeComplete && s.snakeApproaching);
            drawShadowDisc(s.snakeX, s.snakeZ, 0.4f);
            break;
        }

        case STAGE_FIRST_AID: {
            drawForestEnvironment();
            // Seated patient
            glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.2f);
            drawHuman(true, false, s.limbImmobilized, true, s.simTime, 0.0f, 0.18f, 0.46f, 0.72f);
            glPopMatrix();
            drawShadowDisc(0.0f, 0.2f, 0.5f);
            // Responder (player)
            glPushMatrix();
            glTranslatef(s.playerX, 0.0f, s.playerZ);
            glRotatef(s.playerAngle, 0.0f, 1.0f, 0.0f);
            drawHuman(false, false, false, false, s.simTime,
                     s.playerMoving ? s.playerWalkPhase : 0.0f, 0.85f, 0.35f, 0.15f);
            glPopMatrix();
            drawShadowDisc(s.playerX, s.playerZ, 0.5f);
            // Snake in distance
            drawSnake(s, -3.2f, 0.0f, -3.0f, 35.0f, false);
            break;
        }

        case STAGE_HANDLING: {
            drawForestEnvironment();
            // Extraction zone — ONLY in Stage 3
            drawExtractionZone(s);

            float cx = s.onStretcher && s.stretcherLiftProgress >= 1.0f ? s.stretcherCarryX : 1.65f;
            float cz = s.onStretcher && s.stretcherLiftProgress >= 1.0f ? s.stretcherCarryZ : 0.0f;
            drawStretcher(cx, 0.0f, cz);
            float p = s.stretcherLiftProgress;
            float px = lerpf(0.0f, cx, p);
            float py = 0.62f * p;
            // Patient centered on stretcher mattress
            glPushMatrix(); glTranslatef(px, py, cz);
            drawHuman(!s.onStretcher, s.onStretcher, true, true, s.simTime, 0.0f, 0.18f, 0.46f, 0.72f);
            glPopMatrix();
            // Responder at stretcher rear handles with forward-angled arms
            if (s.onStretcher && s.stretcherLiftProgress >= 1.0f) {
                glPushMatrix();
                glTranslatef(cx, 0.0f, cz + 1.6f); // behind stretcher (higher Z)
                glRotatef(180.0f, 0.0f, 1.0f, 0.0f); // face forward
                drawHuman(false, false, false, false, s.simTime,
                         s.playerMoving ? s.playerWalkPhase : 0.0f, 0.85f, 0.35f, 0.15f);
                glPopMatrix();

                // Floating green arrow pointing toward extraction zone
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                float arrowBob = std::sin(s.simTime * 4.0f) * 0.15f;
                float arrowY = 2.8f + arrowBob;
                glColor4f(0.2f, 1.0f, 0.4f, 0.75f);
                // Arrow body
                glLineWidth(3.0f);
                glBegin(GL_LINES);
                glVertex3f(cx, arrowY, cz);
                glVertex3f(cx, arrowY, cz - 2.0f);
                glEnd();
                // Arrow head
                glBegin(GL_TRIANGLES);
                glVertex3f(cx, arrowY, cz - 2.4f);
                glVertex3f(cx - 0.4f, arrowY, cz - 1.6f);
                glVertex3f(cx + 0.4f, arrowY, cz - 1.6f);
                glEnd();
                glLineWidth(1.0f);
                glDisable(GL_BLEND);
                glEnable(GL_LIGHTING);
            }
            break;
        }

        case STAGE_TRANSPORT:
            drawAmbulanceScene(s);
            break;

        case STAGE_HOSPITAL:
            drawHospitalERScene(s);
            break;
    }

    if (s.currentStage >= STAGE_RECOGNITION && s.currentStage <= STAGE_HANDLING)
        glDisable(GL_FOG);

    // Render particles
    renderParticles(s.particles);
}

} // namespace Models3D

#endif // MODELS_3D_H
