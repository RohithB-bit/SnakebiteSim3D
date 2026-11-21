#ifndef HUD_2D_H
#define HUD_2D_H

#include <windows.h>
#include <GL/freeglut.h>
#include <string>
#include <cstdio>
#include <cmath>
#include "sim_state.h"

namespace HUD2D {

inline void drawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

inline void drawRectBorder(float x, float y, float w, float h, float r, float g, float b, float a, float lineW = 1.5f) {
    glLineWidth(lineW);
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);
}

inline void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18,
                     float r = 1.0f, float g = 1.0f, float b = 1.0f) {
    glColor4f(r, g, b, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

inline int textWidth(const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    return glutBitmapLength(font, reinterpret_cast<const unsigned char*>(text.c_str()));
}

inline void drawLiveECG(float x, float y, float w, float h, float ecgPhase, float heartRate) {
    drawRect(x, y, w, h, 0.03f, 0.09f, 0.07f, 0.92f);
    drawRectBorder(x, y, w, h, 0.14f, 0.55f, 0.36f, 0.85f, 1.0f);
    glColor4f(0.10f, 0.32f, 0.22f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(x + 2.0f, y + h * 0.5f); glVertex2f(x + w - 2.0f, y + h * 0.5f);
    glEnd();
    if (heartRate > 130.0f) glColor4f(1.0f, 0.28f, 0.28f, 1.0f);
    else if (heartRate > 105.0f) glColor4f(1.0f, 0.78f, 0.22f, 1.0f);
    else glColor4f(0.18f, 0.96f, 0.48f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 64; i++) {
        float u = (float)i / 64.0f;
        float px = x + 4.0f + u * (w - 8.0f);
        float p = std::fmod(u * 2.5f + ecgPhase * 0.45f, 1.0f);
        float amp = 0.0f;
        if (p > 0.36f && p < 0.41f) amp = 0.16f;
        else if (p >= 0.41f && p < 0.45f) amp = -0.18f;
        else if (p >= 0.45f && p < 0.50f) amp = 0.44f;
        else if (p >= 0.50f && p < 0.54f) amp = -0.26f;
        else if (p >= 0.54f && p < 0.62f) amp = 0.14f;
        glVertex2f(px, y + h * 0.5f + amp * (h - 8.0f));
    }
    glEnd();
    glLineWidth(1.0f);
}

// ============================================================
// VIGNETTE EFFECT (Red edge flash)
// ============================================================
inline void drawVignette(float intensity, int w, int h) {
    if (intensity < 0.01f) return;
    float a = intensity * 0.6f;
    // Top
    glBegin(GL_QUADS);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f(0, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f((float)w, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f((float)w, h - 80.0f);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(0, h - 80.0f);
    glEnd();
    // Bottom
    glBegin(GL_QUADS);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f(0, 0);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f((float)w, 0);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f((float)w, 80.0f);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(0, 80.0f);
    glEnd();
    // Left
    glBegin(GL_QUADS);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f(0, 0);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f(0, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(60.0f, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(60.0f, 0);
    glEnd();
    // Right
    glBegin(GL_QUADS);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f((float)w, 0);
    glColor4f(0.9f, 0.05f, 0.05f, a); glVertex2f((float)w, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(w - 60.0f, (float)h);
    glColor4f(0.9f, 0.05f, 0.05f, 0.0f); glVertex2f(w - 60.0f, 0);
    glEnd();
}

// ============================================================
// SPLINT TENSION METER (Stage 2 mini-game)
// ============================================================
inline void drawTensionMeter(const SimState& s, float cx, float cy) {
    float mw = 280.0f, mh = 50.0f;
    float mx = cx - mw * 0.5f, my = cy;
    // Background
    drawRect(mx, my, mw, mh, 0.08f, 0.10f, 0.14f, 0.92f);
    drawRectBorder(mx, my, mw, mh, 0.6f, 0.7f, 0.8f, 0.9f, 2.0f);

    // Red zone (too loose) - left
    drawRect(mx + 4, my + 4, mw * 0.3f - 4, mh - 8, 0.8f, 0.2f, 0.2f, 0.4f);
    // GREEN SAFE ZONE - center
    drawRect(mx + mw * 0.3f, my + 4, mw * 0.3f, mh - 8, 0.15f, 0.8f, 0.3f, 0.5f);
    // Red zone (too tight) - right
    drawRect(mx + mw * 0.6f, my + 4, mw * 0.4f - 4, mh - 8, 0.8f, 0.15f, 0.15f, 0.4f);

    // Labels
    drawText(mx + 15, my + mh * 0.4f, "LOOSE", GLUT_BITMAP_HELVETICA_12, 0.9f, 0.5f, 0.5f);
    drawText(mx + mw * 0.35f, my + mh * 0.4f, "SAFE", GLUT_BITMAP_HELVETICA_12, 0.3f, 1.0f, 0.5f);
    drawText(mx + mw * 0.68f, my + mh * 0.4f, "TIGHT!", GLUT_BITMAP_HELVETICA_12, 1.0f, 0.3f, 0.3f);

    // Moving needle
    float needleX = mx + 4.0f + s.tensionNeedle * (mw - 8.0f);
    glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(needleX, my + 2.0f);
    glVertex2f(needleX, my + mh - 2.0f);
    glEnd();
    glLineWidth(1.0f);

    drawText(mx, my + mh + 6, "PRESS [SPACE] IN GREEN ZONE!", GLUT_BITMAP_HELVETICA_12, 1.0f, 1.0f, 0.5f);
    drawText(mx, my - 16, "SPLINT TENSION METER", GLUT_BITMAP_HELVETICA_18, 0.5f, 0.9f, 1.0f);
}

// ============================================================
// IV DRIP FLOW GAUGE (Stage 5 mini-game)
// ============================================================
inline void drawIVFlowGauge(const SimState& s, float cx, float cy) {
    float gw = 30.0f, gh = 200.0f;
    float gx = cx - gw * 0.5f, gy = cy;
    // Background
    drawRect(gx, gy, gw, gh, 0.08f, 0.10f, 0.14f, 0.92f);
    drawRectBorder(gx, gy, gw, gh, 0.5f, 0.7f, 0.9f, 0.9f, 2.0f);

    // Green optimal zone
    float zoneBottom = gy + gh * s.ivOptimalLow;
    float zoneTop = gy + gh * s.ivOptimalHigh;
    drawRect(gx + 2, zoneBottom, gw - 4, zoneTop - zoneBottom, 0.15f, 0.8f, 0.3f, 0.45f);

    // Flow rate indicator
    float flowY = gy + gh * s.ivFlowRate;
    bool inZone = (s.ivFlowRate >= s.ivOptimalLow && s.ivFlowRate <= s.ivOptimalHigh);
    if (inZone) glColor4f(0.2f, 1.0f, 0.4f, 1.0f);
    else glColor4f(1.0f, 0.3f, 0.3f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(gx - 4, flowY);
    glVertex2f(gx + gw + 4, flowY);
    glEnd();
    // Arrow
    glBegin(GL_TRIANGLES);
    glVertex2f(gx - 10, flowY);
    glVertex2f(gx - 4, flowY + 5);
    glVertex2f(gx - 4, flowY - 5);
    glEnd();
    glLineWidth(1.0f);

    // Labels
    drawText(gx + gw + 8, gy + gh - 10, "FAST", GLUT_BITMAP_HELVETICA_12, 0.9f, 0.5f, 0.5f);
    drawText(gx + gw + 8, (zoneBottom + zoneTop) * 0.5f - 4, "OPTIMAL", GLUT_BITMAP_HELVETICA_12, 0.3f, 1.0f, 0.5f);
    drawText(gx + gw + 8, gy + 4, "SLOW", GLUT_BITMAP_HELVETICA_12, 0.5f, 0.5f, 0.9f);

    drawText(gx - 50, gy + gh + 10, "IV DRIP", GLUT_BITMAP_HELVETICA_12, 0.5f, 0.9f, 1.0f);
    drawText(gx - 55, gy - 18, "W/S: ADJUST", GLUT_BITMAP_HELVETICA_12, 0.8f, 0.8f, 0.6f);
}

// ============================================================
// TITLE OVERLAY
// ============================================================
inline void renderTitleOverlay(const SimState& s, int w, int h) {
    float panelW = 540.0f, panelH = 500.0f;
    float px = 48.0f, py = (h - panelH) * 0.5f;
    drawRect(px, py, panelW, panelH, 0.06f, 0.09f, 0.14f, 0.88f);
    drawRectBorder(px, py, panelW, panelH, 0.24f, 0.62f, 0.88f, 0.85f, 2.0f);
    drawRect(px, py + panelH - 6.0f, panelW, 6.0f, 0.88f, 0.18f, 0.22f, 0.95f);

    // 4 Corner Accent Brackets (Glowing Cyan)
    float arm = 18.0f;
    float off = 3.0f;
    float cornerPulse = 0.8f + 0.2f * std::sin(s.simTime * 3.5f);
    glColor4f(0.15f, 0.90f, 0.98f, 0.85f * cornerPulse);
    glLineWidth(2.5f);
    // Top-Left
    glBegin(GL_LINE_STRIP);
    glVertex2f(px - off, py + panelH - arm);
    glVertex2f(px - off, py + panelH + off);
    glVertex2f(px + arm, py + panelH + off);
    glEnd();
    // Top-Right
    glBegin(GL_LINE_STRIP);
    glVertex2f(px + panelW - arm, py + panelH + off);
    glVertex2f(px + panelW + off, py + panelH + off);
    glVertex2f(px + panelW + off, py + panelH - arm);
    glEnd();
    // Bottom-Left
    glBegin(GL_LINE_STRIP);
    glVertex2f(px - off, py + arm);
    glVertex2f(px - off, py - off);
    glVertex2f(px + arm, py - off);
    glEnd();
    // Bottom-Right
    glBegin(GL_LINE_STRIP);
    glVertex2f(px + panelW - arm, py - off);
    glVertex2f(px + panelW + off, py - off);
    glVertex2f(px + panelW + off, py + arm);
    glEnd();
    glLineWidth(1.0f);

    drawText(px + 28, py + panelH - 46, "3D SNAKEBITE EMERGENCY", GLUT_BITMAP_TIMES_ROMAN_24, 0.98f, 0.98f, 1.0f);
    drawText(px + 28, py + panelH - 74, "MANAGEMENT & FIRST-AID SIMULATOR", GLUT_BITMAP_HELVETICA_18, 0.38f, 0.82f, 0.98f);

    // Subtle animated mini ECG heartbeat trace along divider line
    float divX = px + 28.0f;
    float divY = py + panelH - 92.0f;
    float divW = panelW - 56.0f;
    drawRect(divX, divY - 0.5f, divW, 1.5f, 0.20f, 0.32f, 0.45f, 0.6f);

    glColor4f(0.18f, 0.96f, 0.52f, 0.85f);
    glLineWidth(1.8f);
    glBegin(GL_LINE_STRIP);
    const int ecgN = 56;
    for (int i = 0; i <= ecgN; i++) {
        float u = (float)i / ecgN;
        float ex = divX + u * divW;
        float p = std::fmod(u * 2.2f + s.simTime * 0.45f, 1.0f);
        float amp = 0.0f;
        if (p > 0.40f && p < 0.44f) amp = 2.0f;
        else if (p >= 0.44f && p < 0.47f) amp = -2.5f;
        else if (p >= 0.47f && p < 0.52f) amp = 8.5f;
        else if (p >= 0.52f && p < 0.56f) amp = -4.0f;
        else if (p >= 0.56f && p < 0.63f) amp = 2.2f;
        glVertex2f(ex, divY + amp);
    }
    glEnd();
    glLineWidth(1.0f);

    drawText(px + 28, py + panelH - 122, "PLAYABLE CLINICAL STAGES:", GLUT_BITMAP_HELVETICA_12, 0.65f, 0.75f, 0.86f);

    const char* stages[] = {
        "1. Walk the Forest Trail & Identify the Cobra",
        "2. Precision Splint Mini-Game & First Aid",
        "3. Carry Stretcher to Extraction Zone",
        "4. High-Speed Ambulance Highway Dodge!",
        "5. ER: Regulate IV Drip Flow for ASV Cure"
    };
    for (int i = 0; i < 5; i++) {
        drawText(px + 34, py + panelH - 146 - i * 25, stages[i], GLUT_BITMAP_HELVETICA_18, 0.90f, 0.94f, 0.98f);
    }

    drawRect(px + 28, py + 124, panelW - 56, 68, 0.10f, 0.15f, 0.22f, 0.92f);
    drawText(px + 42, py + 168, "MOVE: WASD/Arrows | CAMERA: Left-Drag + Scroll", GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.94f);
    drawText(px + 42, py + 148, "ACTIONS: [1]-[4] | MENU: Right-Click | [M] Mute", GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.94f);
    drawText(px + 42, py + 128, "[C] Camera Preset | [R] Restart | [ESC] Quit", GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.94f);

    float pulse = 0.75f + 0.25f * std::sin(s.simTime * 4.2f);
    drawRect(px + 28, py + 38, panelW - 56, 52, 0.12f * pulse, 0.62f * pulse, 0.38f * pulse, 0.95f);
    drawRectBorder(px + 28, py + 38, panelW - 56, 52, 0.45f, 0.98f, 0.72f, 1.0f, 2.0f);
    std::string cta = ">>> PRESS [ENTER] TO START <<<";
    int tw = textWidth(cta, GLUT_BITMAP_HELVETICA_18);
    drawText(px + (panelW - tw) * 0.5f, py + 56, cta, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);

    // Group 14 Footer Tag
    std::string footerTag = "GROUP 14 | OPENGL 3D SIMULATION";
    int ftw = textWidth(footerTag, GLUT_BITMAP_HELVETICA_12);
    drawText(px + (panelW - ftw) * 0.5f, py + 14.0f, footerTag, GLUT_BITMAP_HELVETICA_12, 0.45f, 0.68f, 0.85f);
}

// ============================================================
// DEBRIEF OVERLAY WITH RANK
// ============================================================
inline void renderDebriefOverlay(const SimState& s, int w, int h) {
    float panelW = 800.0f, panelH = 560.0f;
    float px = (w - panelW) * 0.5f, py = (h - panelH) * 0.5f;
    drawRect(px, py, panelW, panelH, 0.05f, 0.08f, 0.13f, 0.93f);
    if (s.patientSurvived) drawRectBorder(px, py, panelW, panelH, 0.20f, 0.86f, 0.52f, 0.95f, 2.5f);
    else drawRectBorder(px, py, panelW, panelH, 0.94f, 0.24f, 0.24f, 0.95f, 2.5f);

    std::string outcome = s.patientSurvived ? "PATIENT SURVIVED & STABILIZED" : "CRITICAL ENVENOMATION FAILURE";
    float or_ = s.patientSurvived ? 0.24f : 1.0f;
    float og = s.patientSurvived ? 0.96f : 0.30f;
    float ob = s.patientSurvived ? 0.58f : 0.30f;
    drawText(px + 32, py + panelH - 46, outcome, GLUT_BITMAP_TIMES_ROMAN_24, or_, og, ob);

    // CLINICAL RANK
    char buf[128];
    std::string rank = s.clinicalRank;
    float rankSize = 1.0f;
    float rr = 0.9f, rg = 0.9f, rb = 0.3f;
    if (rank == "S") { rr = 1.0f; rg = 0.85f; rb = 0.0f; }
    else if (rank == "A") { rr = 0.3f; rg = 1.0f; rb = 0.4f; }
    else if (rank == "B") { rr = 0.3f; rg = 0.7f; rb = 1.0f; }
    else if (rank == "C") { rr = 0.8f; rg = 0.6f; rb = 0.2f; }
    else { rr = 0.9f; rg = 0.2f; rb = 0.2f; }

    float rankX = px + panelW - 120;
    float rankY = py + panelH - 110;
    drawRect(rankX - 10, rankY - 30, 100, 90, 0.08f, 0.10f, 0.15f, 0.95f);
    drawRectBorder(rankX - 10, rankY - 30, 100, 90, rr, rg, rb, 1.0f, 3.0f);
    drawText(rankX + 10, rankY + 24, "RANK", GLUT_BITMAP_HELVETICA_12, 0.7f, 0.7f, 0.8f);
    drawText(rankX + 20, rankY - 10, rank, GLUT_BITMAP_TIMES_ROMAN_24, rr, rg, rb);

    int finalScore = s.computeFinalScore();
    drawRect(px + 32, py + panelH - 134, panelW - 180, 68, 0.10f, 0.14f, 0.21f, 0.95f);
    std::snprintf(buf, sizeof(buf), "FINAL SCORE: %d / 100", finalScore);
    drawText(px + 52, py + panelH - 96, buf, GLUT_BITMAP_HELVETICA_18, 1.0f, 0.90f, 0.30f);
    std::snprintf(buf, sizeof(buf), "TIME: %.1fs", s.elapsedTime);
    drawText(px + 310, py + panelH - 96, buf, GLUT_BITMAP_HELVETICA_18, 0.85f, 0.92f, 1.0f);
    std::snprintf(buf, sizeof(buf), "OK: %d | ERR: %d", s.correctDecisions, s.wrongDecisions);
    drawText(px + 450, py + panelH - 96, buf, GLUT_BITMAP_HELVETICA_18, 0.78f, 0.94f, 0.85f);
    std::snprintf(buf, sizeof(buf), "Venom: %.1f%%  |  HR: %.0f BPM  |  Dodge Combo: %d  %s",
                  s.venomLevel, s.heartRate, s.maxDodgeCombo,
                  s.zeroMistakes ? "| ZERO MISTAKES!" : "");
    drawText(px + 52, py + panelH - 120, buf, GLUT_BITMAP_HELVETICA_12, 0.70f, 0.80f, 0.90f);

    drawText(px + 32, py + panelH - 158, "CLINICAL DECISION AUDIT LOG:", GLUT_BITMAP_HELVETICA_18, 0.45f, 0.82f, 0.98f);

    float rowY = py + panelH - 188;
    int maxRows = 7;
    int startIdx = ((int)s.decisionLog.size() > maxRows) ? (int)s.decisionLog.size() - maxRows : 0;
    if (s.decisionLog.empty()) {
        drawText(px + 42, rowY, "No actions recorded (jumped to debrief).", GLUT_BITMAP_HELVETICA_12, 0.72f, 0.76f, 0.82f);
    } else {
        for (int i = startIdx; i < (int)s.decisionLog.size(); i++) {
            const auto& rec = s.decisionLog[i];
            std::string badge = rec.isCorrect ? "[OK] " : "[ERR] ";
            float cr = rec.isCorrect ? 0.30f : 1.0f;
            float cg = rec.isCorrect ? 0.95f : 0.36f;
            float cb = rec.isCorrect ? 0.56f : 0.36f;
            drawText(px + 36, rowY, badge + rec.action, GLUT_BITMAP_HELVETICA_12, cr, cg, cb);
            drawText(px + 58, rowY - 16, rec.clinicalNote, GLUT_BITMAP_HELVETICA_12, 0.76f, 0.82f, 0.88f);
            rowY -= 42.0f;
        }
    }

    drawRect(px + 32, py + 24, panelW - 64, 44, 0.14f, 0.24f, 0.36f, 0.92f);
    drawRectBorder(px + 32, py + 24, panelW - 64, 44, 0.38f, 0.74f, 0.98f, 0.9f, 1.5f);
    std::string rt = "[R] RESTART  |  RIGHT-CLICK: JUMP STAGES  |  [ESC] EXIT";
    int rtw = textWidth(rt, GLUT_BITMAP_HELVETICA_12);
    drawText(px + (panelW - rtw) * 0.5f, py + 41, rt, GLUT_BITMAP_HELVETICA_12, 0.95f, 0.98f, 1.0f);
}

// ============================================================
// ACTIVE GAMEPLAY HUD
// ============================================================
inline void renderActiveHUD(const SimState& s, int w, int h) {
    char buf[256];

    // TOP BAR
    float topH = 74.0f;
    drawRect(0, h - topH, (float)w, topH, 0.05f, 0.08f, 0.12f, 0.86f);
    drawRect(0, h - topH, (float)w, 2.0f, 0.20f, 0.52f, 0.78f, 0.90f);
    drawText(20, h - 30, s.getStageName(), GLUT_BITMAP_HELVETICA_18, 0.42f, 0.86f, 1.0f);

    std::snprintf(buf, sizeof(buf), "TIME: %02d:%04.1f  |  SCORE: %d  |  STABILITY: %.0f/100",
                  (int)s.elapsedTime / 60, std::fmod(s.elapsedTime, 60.0f),
                  s.score, s.stabilityScore);
    drawText(20, h - 56, buf, GLUT_BITMAP_HELVETICA_12, 0.84f, 0.90f, 0.95f);

    // Venom bar
    float barW = 260.0f, barH = 18.0f;
    float barX = w * 0.5f - barW * 0.5f + 20.0f, barY = h - 56.0f;
    std::snprintf(buf, sizeof(buf), "VENOM: %.1f%%", s.venomLevel);
    drawText(barX, h - 28, buf, GLUT_BITMAP_HELVETICA_12, 0.95f, 0.95f, 0.95f);
    drawRect(barX, barY, barW, barH, 0.14f, 0.16f, 0.20f, 0.95f);
    float fillW = barW * clampf(s.venomLevel / 100.0f, 0.0f, 1.0f);
    float vr = (s.venomLevel < 40) ? 0.22f : ((s.venomLevel < 70) ? 0.96f : 0.95f);
    float vg = (s.venomLevel < 40) ? 0.82f : ((s.venomLevel < 70) ? 0.74f : 0.18f);
    float vb = (s.venomLevel < 40) ? 0.42f : 0.16f;
    drawRect(barX, barY, fillW, barH, vr, vg, vb, 0.95f);
    drawRectBorder(barX, barY, barW, barH, 0.65f, 0.72f, 0.80f, 0.8f, 1.0f);

    // BPM + ECG
    float ecgW = 155.0f, ecgH = 48.0f;
    float ecgX = w - ecgW - 20.0f, ecgY = h - 61.0f;
    drawLiveECG(ecgX, ecgY, ecgW, ecgH, s.ecgPhase, s.heartRate);
    std::snprintf(buf, sizeof(buf), "%.0f BPM", s.heartRate);
    float hrR = (s.heartRate > 125) ? 1.0f : 0.30f;
    float hrG = (s.heartRate > 125) ? 0.35f : 0.96f;
    float hrB = (s.heartRate > 125) ? 0.35f : 0.56f;
    drawText(ecgX - 88, h - 42, buf, GLUT_BITMAP_HELVETICA_18, hrR, hrG, hrB);

    // Mute indicator with dark pill badge
    if (AudioSynth::isMuted()) {
        drawRect((float)w - 108, h - 94, 100, 22, 0.12f, 0.08f, 0.08f, 0.85f);
        drawText((float)w - 95, h - 86, "[M] MUTED", GLUT_BITMAP_HELVETICA_12, 0.95f, 0.4f, 0.4f);
    } else {
        drawRect((float)w - 108, h - 94, 100, 22, 0.08f, 0.10f, 0.14f, 0.70f);
        drawText((float)w - 95, h - 86, "[M] Audio", GLUT_BITMAP_HELVETICA_12, 0.5f, 0.6f, 0.65f);
    }

    // STAGE-SPECIFIC HUD ELEMENTS

    // Stage 2: Tension meter
    if (s.currentStage == STAGE_FIRST_AID && s.splintMiniGameActive && !s.splintSuccess) {
        drawTensionMeter(s, w * 0.5f, h * 0.42f);
    }

    // Stage 4: Distance + dodge combo
    if (s.currentStage == STAGE_TRANSPORT) {
        float dw = 380.0f, dx = (w - dw) * 0.5f, dy = h - 114.0f;
        drawRect(dx, dy, dw, 32, 0.06f, 0.09f, 0.14f, 0.86f);
        drawRectBorder(dx, dy, dw, 32, 0.95f, 0.32f, 0.32f, 0.9f, 1.5f);
        float progress = clampf(1.0f - (s.distanceToHospital / 8.0f), 0.0f, 1.0f);
        drawRect(dx + 4, dy + 4, (dw - 8) * progress, 24, 0.18f, 0.68f, 0.92f, 0.55f);
        std::snprintf(buf, sizeof(buf), "HOSPITAL: %.2f km  %s  |  COMBO: x%d",
                      s.distanceToHospital, s.sirenActive ? "[SIREN]" : "[NORMAL]", s.dodgeCombo);
        drawText(dx + 14, dy + 11, buf, GLUT_BITMAP_HELVETICA_12, 1.0f, 1.0f, 1.0f);

        // Lane indicator — larger with labels
        float laneY = 20.0f;
        float laneBaseX = w * 0.5f - 72;
        drawText(laneBaseX, laneY, "A/D: CHANGE LANE", GLUT_BITMAP_HELVETICA_12, 0.7f, 0.8f, 0.9f);
        const char* laneLabels[] = {"L", "C", "R"};
        for (int l = 0; l < 3; l++) {
            float lx = laneBaseX + l * 48;
            if (l == s.ambulanceLane) {
                drawRect(lx, laneY + 20, 38, 32, 0.15f, 0.85f, 0.3f, 0.95f);
                drawRectBorder(lx, laneY + 20, 38, 32, 0.4f, 1.0f, 0.5f, 1.0f, 2.0f);
                drawText(lx + 13, laneY + 30, laneLabels[l], GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
            } else {
                drawRect(lx, laneY + 20, 38, 32, 0.2f, 0.2f, 0.25f, 0.6f);
                drawRectBorder(lx, laneY + 20, 38, 32, 0.5f, 0.5f, 0.6f, 0.7f, 1.0f);
                drawText(lx + 13, laneY + 30, laneLabels[l], GLUT_BITMAP_HELVETICA_18, 0.6f, 0.6f, 0.7f);
            }
        }
    }

    // Stage 5: IV flow gauge
    if (s.currentStage == STAGE_HOSPITAL && s.asvAdministered) {
        drawIVFlowGauge(s, w - 90.0f, h * 0.25f);
    }

    // FEEDBACK TOAST
    if (s.feedbackTimer > 0.0f && !s.feedbackMsg.empty()) {
        float fw = (float)w - 48.0f, fh = 54.0f, fx = 24.0f, fy = 148.0f;
        float br = 0.24f, bg = 0.64f, bb = 0.96f;
        if (s.feedbackType == 1) { br = 0.20f; bg = 0.86f; bb = 0.48f; }
        if (s.feedbackType == 2) { br = 0.96f; bg = 0.24f; bb = 0.24f; }
        drawRect(fx, fy, fw, fh, 0.06f, 0.08f, 0.12f, 0.88f);
        drawRect(fx, fy, 6.0f, fh, br, bg, bb, 1.0f);
        drawRectBorder(fx, fy, fw, fh, br, bg, bb, 0.85f, 1.5f);
        drawText(fx + 18, fy + 32, s.feedbackTitle, GLUT_BITMAP_HELVETICA_18, br, bg, bb);
        drawText(fx + 18, fy + 11, s.feedbackMsg, GLUT_BITMAP_HELVETICA_12, 0.94f, 0.96f, 0.98f);
    }

    // BOTTOM CONTROLS PANEL
    float pw = (float)w - 48.0f, ph = 122.0f, ppx = 24.0f, ppy = 14.0f;
    drawRect(ppx, ppy, pw, ph, 0.05f, 0.08f, 0.13f, 0.88f);
    drawRectBorder(ppx, ppy, pw, ph, 0.28f, 0.44f, 0.62f, 0.85f, 1.5f);

    drawText(ppx + 18, ppy + ph - 22, "CLINICAL ACTIONS:", GLUT_BITMAP_HELVETICA_12, 0.60f, 0.82f, 0.98f);
    std::string hint = "WASD: Move | Drag: Orbit | [C] Cam | R-Click: Menu";
    int hintW = textWidth(hint, GLUT_BITMAP_HELVETICA_12);
    drawText(ppx + pw - 18.0f - hintW, ppy + ph - 22, hint, GLUT_BITMAP_HELVETICA_12, 0.55f, 0.62f, 0.72f);

    float lc = ppx + 22, rc = ppx + pw * 0.50f;
    float r1 = ppy + 62, r2 = ppy + 26;

    if (s.currentStage == STAGE_RECOGNITION) {
        if (!s.strikeComplete) {
            drawText(lc, r1, "Walk forward with WASD toward the trail bend...", GLUT_BITMAP_HELVETICA_18, 0.7f, 0.8f, 0.9f);
        } else {
            drawText(lc, r1, "[1] Identify Venomous Cobra (Protocol)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
            drawText(rc, r1, "[2] Non-Venomous (Observe)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
            drawText(lc, r2, "[3] Chase / Kill Snake", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
        }
    } else if (s.currentStage == STAGE_FIRST_AID) {
        std::string s1 = s.limbImmobilized ? "[1] Splint [DONE]" : "[1] Immobilize Limb (Splint)";
        std::string s4 = s.patientCalmed ? "[4] Calm & 108 [DONE]" : "[4] Calm Patient & Call 108";
        drawText(lc, r1, s1, GLUT_BITMAP_HELVETICA_18, s.limbImmobilized ? 0.35f : 0.95f, s.limbImmobilized ? 0.95f : 0.96f, s.limbImmobilized ? 0.55f : 1.0f);
        drawText(rc, r1, "[2] Tourniquet (MYTH!)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
        drawText(lc, r2, "[3] Cut & Suck (MYTH!)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
        drawText(rc, r2, s4, GLUT_BITMAP_HELVETICA_18, s.patientCalmed ? 0.35f : 0.95f, s.patientCalmed ? 0.95f : 0.96f, s.patientCalmed ? 0.55f : 1.0f);
    } else if (s.currentStage == STAGE_HANDLING) {
        drawText(lc, r1, "[1] Load Patient onto Stretcher", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
        drawText(lc, r2, "[2] Make Patient Walk (DANGEROUS!)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
        if (s.onStretcher && s.stretcherLiftProgress >= 1.0f)
            drawText(rc, r1, "WASD: Carry to GREEN extraction zone!", GLUT_BITMAP_HELVETICA_18, 0.3f, 1.0f, 0.5f);
    } else if (s.currentStage == STAGE_TRANSPORT) {
        std::string t1 = s.sirenActive ? "[1] Siren [ACTIVE]" : "[1] Activate Siren";
        std::string t2 = s.vitalsMonitored ? "[2] Vitals [DONE]" : "[2] Monitor Vitals";
        drawText(lc, r1, t1, GLUT_BITMAP_HELVETICA_18, s.sirenActive ? 0.35f : 0.95f, s.sirenActive ? 0.95f : 0.96f, s.sirenActive ? 0.55f : 1.0f);
        drawText(rc, r1, t2, GLUT_BITMAP_HELVETICA_18, s.vitalsMonitored ? 0.35f : 0.95f, s.vitalsMonitored ? 0.95f : 0.96f, s.vitalsMonitored ? 0.55f : 1.0f);
        drawText(lc, r2, "A/D: Dodge cars!  Avoid collisions!", GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.5f);
    } else if (s.currentStage == STAGE_HOSPITAL) {
        std::string h1 = s.asvAdministered ? "[1] ASV Infusing... (Regulate W/S)" : "[1] Administer ASV IV";
        drawText(lc, r1, h1, GLUT_BITMAP_HELVETICA_18, s.asvAdministered ? 0.35f : 0.95f, s.asvAdministered ? 0.95f : 0.96f, s.asvAdministered ? 0.55f : 1.0f);
        drawText(lc, r2, "[2] Delay ASV / Painkiller (WRONG!)", GLUT_BITMAP_HELVETICA_18, 0.94f, 0.96f, 1.0f);
    }
}

// ============================================================
// MASTER HUD RENDERER
// ============================================================
inline void renderHUD(const SimState& s, int winW, int winH) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, (double)winW, 0.0, (double)winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (s.currentStage == STAGE_TITLE)
        renderTitleOverlay(s, winW, winH);
    else if (s.currentStage == STAGE_DEBRIEF)
        renderDebriefOverlay(s, winW, winH);
    else
        renderActiveHUD(s, winW, winH);

    // Red vignette overlay
    drawVignette(s.vignetteFlash, winW, winH);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

} // namespace HUD2D

#endif // HUD_2D_H
