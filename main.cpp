#include <windows.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include "audio_synth.h"
#include "sim_state.h"
#include "models_3d.h"
#include "hud_2d.h"

SimState gSim;
int gWinW = 1280;
int gWinH = 720;

bool gLeftDragging = false;
int gLastMouseX = 0;
int gLastMouseY = 0;
int gLastFrameMs = 0;

// WASD movement key states
bool gKeyW = false, gKeyA = false, gKeyS = false, gKeyD = false;
bool gArrowUp = false, gArrowDown = false, gArrowLeft = false, gArrowRight = false;

void setupCamera() {
    float yawRad   = gSim.camYaw   * (3.14159265f / 180.0f);
    float pitchRad = gSim.camPitch * (3.14159265f / 180.0f);

    float targetY = 1.25f;
    float targetX = 0.0f;
    float targetZ = 0.0f;

    if (gSim.currentStage == STAGE_TITLE || gSim.currentStage == STAGE_DEBRIEF) {
        targetY = 1.45f;
        targetX = -2.2f;
    } else if (gSim.currentStage == STAGE_FIRST_AID && gSim.camPreset == 1) {
        targetY = 0.65f;
    } else if (gSim.currentStage == STAGE_RECOGNITION ||
               gSim.currentStage == STAGE_FIRST_AID ||
               gSim.currentStage == STAGE_HANDLING) {
        // Camera follows player
        targetX = gSim.playerX;
        targetZ = gSim.playerZ;
        targetY = 1.5f;
        if (gSim.currentStage == STAGE_HANDLING && gSim.onStretcher && gSim.stretcherLiftProgress >= 1.0f) {
            targetX = gSim.stretcherCarryX;
            targetZ = gSim.stretcherCarryZ;
        }
    } else if (gSim.currentStage == STAGE_TRANSPORT) {
        targetX = gSim.ambulanceActualX;
        targetY = 2.0f;
    }

    float eyeX = targetX + gSim.camDist * std::cos(pitchRad) * std::sin(yawRad);
    float eyeY = targetY + gSim.camDist * std::sin(pitchRad);
    float eyeZ = targetZ + gSim.camDist * std::cos(pitchRad) * std::cos(yawRad);

    // Apply camera shake
    eyeX += gSim.camOffsetX;
    eyeY += gSim.camOffsetY;

    gluLookAt(eyeX, eyeY, eyeZ,
              targetX, targetY, targetZ,
              0.0f, 1.0f, 0.0f);
}

void processMovement(float dt) {
    // Determine movement intent
    bool moveForward = gKeyW || gArrowUp;
    bool moveBack    = gKeyS || gArrowDown;
    bool moveLeft    = gKeyA || gArrowLeft;
    bool moveRight   = gKeyD || gArrowRight;

    // Stage 4: Ambulance lane switching (A/D only)
    if (gSim.currentStage == STAGE_TRANSPORT) {
        gSim.playerMoving = false;
        return;
    }

    // Stage 5: IV flow rate control (W/S)
    if (gSim.currentStage == STAGE_HOSPITAL && gSim.asvAdministered) {
        if (moveForward) gSim.ivFlowRate = clampf(gSim.ivFlowRate + dt * 0.8f, 0.0f, 1.0f);
        if (moveBack)    gSim.ivFlowRate = clampf(gSim.ivFlowRate - dt * 0.8f, 0.0f, 1.0f);
        gSim.playerMoving = false;
        return;
    }

    // Player WASD movement for stages 1-3
    if (gSim.currentStage >= STAGE_RECOGNITION && gSim.currentStage <= STAGE_HANDLING) {
        float speed = 3.5f; // units per second
        float dx = 0.0f, dz = 0.0f;

        // Movement relative to camera yaw for intuitive control
        float camRad = gSim.camYaw * (3.14159265f / 180.0f);
        float fwdX = std::sin(camRad);
        float fwdZ = std::cos(camRad);
        float rightX = std::cos(camRad);
        float rightZ = -std::sin(camRad);

        if (moveForward) { dx -= fwdX; dz -= fwdZ; }
        if (moveBack)    { dx += fwdX; dz += fwdZ; }
        if (moveLeft)    { dx -= rightX; dz -= rightZ; }
        if (moveRight)   { dx += rightX; dz += rightZ; }

        float mag = std::sqrt(dx * dx + dz * dz);
        if (mag > 0.01f) {
            dx /= mag; dz /= mag;
            gSim.playerMoving = true;
            gSim.playerSpeed = speed;
            gSim.playerAngle = std::atan2(dx, dz) * (180.0f / 3.14159265f);

            if (gSim.currentStage == STAGE_HANDLING && gSim.onStretcher && gSim.stretcherLiftProgress >= 1.0f) {
                // Move stretcher
                float spd = speed * 0.6f; // slower with stretcher
                gSim.stretcherCarryX += dx * spd * dt;
                gSim.stretcherCarryZ += dz * spd * dt;
                // Clamp to trail
                gSim.stretcherCarryX = clampf(gSim.stretcherCarryX, -2.2f, 2.2f);

                // Rough movement penalty: fast changes spike BPM
                gSim.stretcherBPMPenalty += dt * 0.5f;
                if (gSim.stretcherBPMPenalty > 3.0f) {
                    gSim.targetHeartRate = clampf(gSim.targetHeartRate + 2.0f, 70.0f, 155.0f);
                    gSim.stretcherBPMPenalty = 0.0f;
                }

                gSim.playerX = gSim.stretcherCarryX + 2.0f;
                gSim.playerZ = gSim.stretcherCarryZ;
            } else {
                gSim.playerX += dx * speed * dt;
                gSim.playerZ += dz * speed * dt;
            }
            // Bounds
            gSim.playerX = clampf(gSim.playerX, -15.0f, 15.0f);
            gSim.playerZ = clampf(gSim.playerZ, -20.0f, 20.0f);
        } else {
            gSim.playerMoving = false;
            gSim.playerSpeed = 0.0f;
        }
    }
}

void displayCallback() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setupCamera();
    Models3D::renderScene3D(gSim);
    HUD2D::renderHUD(gSim, gWinW, gWinH);
    glutSwapBuffers();
}

void reshapeCallback(int w, int h) {
    if (h <= 0) h = 1;
    gWinW = w; gWinH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.1, 120.0);
    glMatrixMode(GL_MODELVIEW);
}

void timerCallback(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - gLastFrameMs) / 1000.0f;
    gLastFrameMs = now;
    if (dt <= 0.0f) dt = 0.016f;
    if (dt > 0.05f) dt = 0.05f;

    // Update sound cooldown timer
    AudioSynth::update(dt);

    // Process WASD movement
    processMovement(dt);

    gSim.update(dt);

    // Audio triggers — disable background ECG during Title, Debrief, and Stage 4 Transport
    if (gSim.currentStage != STAGE_TITLE && gSim.currentStage != STAGE_DEBRIEF && gSim.currentStage != STAGE_TRANSPORT) {
        if (gSim.shouldBeepECG(dt)) {
            AudioSynth::playECGBeep();
        }
    }

    // Siren sound loop
    if (gSim.sirenActive && gSim.currentStage == STAGE_TRANSPORT) {
        gSim.sirenSoundTimer += dt;
        if (gSim.sirenSoundTimer > 0.7f) {
            gSim.sirenSoundTimer = 0.0f;
            AudioSynth::playSirenPulse();
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);
}

void keyboardDownCallback(unsigned char key, int, int) {
    // ESC
    if (key == 27) { glutLeaveMainLoop(); return; }

    // ENTER or SPACE
    if (key == 13 || key == ' ') {
        if (gSim.currentStage == STAGE_TITLE) {
            gSim.jumpToStage(STAGE_RECOGNITION);
            AudioSynth::playPositiveChime();
            return;
        }
        // SPACE = splint mini-game hit
        if (key == ' ' && gSim.currentStage == STAGE_FIRST_AID && gSim.splintMiniGameActive) {
            gSim.handleSplintHit();
            if (gSim.splintSuccess) AudioSynth::playPositiveChime();
            else AudioSynth::playErrorBuzz();
            return;
        }
    }

    // R = restart
    if (key == 'r' || key == 'R') { gSim.reset(); return; }

    // C = camera preset cycle
    if (key == 'c' || key == 'C') {
        gSim.applyCameraPreset(gSim.camPreset + 1);
        return;
    }

    // M = mute toggle
    if (key == 'm' || key == 'M') {
        AudioSynth::toggleMute();
        return;
    }

    // E = interact (approach trigger in stage 1)
    if (key == 'e' || key == 'E') {
        if (gSim.currentStage == STAGE_RECOGNITION && !gSim.hikerApproached) {
            gSim.hikerApproached = true;
        }
    }

    // Action keys [1]-[4]
    if (key >= '1' && key <= '4') {
        int choice = key - '0';

        // Audio feedback
        if (gSim.currentStage == STAGE_RECOGNITION) {
            if (choice == 1) AudioSynth::playPositiveChime();
            else { AudioSynth::playErrorBuzz(); }
        } else if (gSim.currentStage == STAGE_FIRST_AID) {
            if (choice == 1 || choice == 4) AudioSynth::playPositiveChime();
            else AudioSynth::playErrorBuzz();
        } else if (gSim.currentStage == STAGE_HANDLING) {
            if (choice == 1) AudioSynth::playPositiveChime();
            else AudioSynth::playErrorBuzz();
        } else if (gSim.currentStage == STAGE_TRANSPORT) {
            AudioSynth::playPositiveChime();
        } else if (gSim.currentStage == STAGE_HOSPITAL) {
            if (choice == 1) AudioSynth::playPositiveChime();
            else AudioSynth::playErrorBuzz();
        }

        gSim.handleActionKey(choice);
        return;
    }

    // WASD key down
    if (key == 'w' || key == 'W') gKeyW = true;
    if (key == 'a' || key == 'A') {
        gKeyA = true;
        // Stage 4: lane change left
        if (gSim.currentStage == STAGE_TRANSPORT && gSim.dodgeCooldown <= 0.0f) {
            if (gSim.ambulanceLane > 0) {
                gSim.ambulanceLane--;
                gSim.dodgeCooldown = 0.15f;
            }
        }
    }
    if (key == 's' || key == 'S') gKeyS = true;
    if (key == 'd' || key == 'D') {
        gKeyD = true;
        // Stage 4: lane change right
        if (gSim.currentStage == STAGE_TRANSPORT && gSim.dodgeCooldown <= 0.0f) {
            if (gSim.ambulanceLane < 2) {
                gSim.ambulanceLane++;
                gSim.dodgeCooldown = 0.15f;
            }
        }
    }

    // Zoom
    if (key == '+' || key == '=') gSim.camTargetDist = clampf(gSim.camTargetDist - 0.8f, 4.0f, 24.0f);
    if (key == '-' || key == '_') gSim.camTargetDist = clampf(gSim.camTargetDist + 0.8f, 4.0f, 24.0f);
}

void keyboardUpCallback(unsigned char key, int, int) {
    if (key == 'w' || key == 'W') gKeyW = false;
    if (key == 'a' || key == 'A') gKeyA = false;
    if (key == 's' || key == 'S') gKeyS = false;
    if (key == 'd' || key == 'D') gKeyD = false;
}

void specialDownCallback(int key, int, int) {
    if (key == GLUT_KEY_UP)    gArrowUp = true;
    if (key == GLUT_KEY_DOWN)  gArrowDown = true;
    if (key == GLUT_KEY_LEFT) {
        gArrowLeft = true;
        if (gSim.currentStage == STAGE_TRANSPORT && gSim.dodgeCooldown <= 0.0f) {
            if (gSim.ambulanceLane > 0) {
                gSim.ambulanceLane--;
                gSim.dodgeCooldown = 0.15f;
            }
        }
    }
    if (key == GLUT_KEY_RIGHT) {
        gArrowRight = true;
        if (gSim.currentStage == STAGE_TRANSPORT && gSim.dodgeCooldown <= 0.0f) {
            if (gSim.ambulanceLane < 2) {
                gSim.ambulanceLane++;
                gSim.dodgeCooldown = 0.15f;
            }
        }
    }
}

void specialUpCallback(int key, int, int) {
    if (key == GLUT_KEY_UP)    gArrowUp = false;
    if (key == GLUT_KEY_DOWN)  gArrowDown = false;
    if (key == GLUT_KEY_LEFT)  gArrowLeft = false;
    if (key == GLUT_KEY_RIGHT) gArrowRight = false;
}

void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        gLeftDragging = (state == GLUT_DOWN);
        gLastMouseX = x; gLastMouseY = y;
    }
}

void motionCallback(int x, int y) {
    if (!gLeftDragging) return;
    int dx = x - gLastMouseX, dy = y - gLastMouseY;
    gLastMouseX = x; gLastMouseY = y;
    gSim.camTargetYaw += dx * 0.45f;
    gSim.camTargetPitch = clampf(gSim.camTargetPitch + dy * 0.35f, 6.0f, 78.0f);
}

void mouseWheelCallback(int, int dir, int, int) {
    if (dir > 0) gSim.camTargetDist = clampf(gSim.camTargetDist - 0.9f, 4.0f, 24.0f);
    else         gSim.camTargetDist = clampf(gSim.camTargetDist + 0.9f, 4.0f, 24.0f);
}

void menuCallback(int option) {
    if (option >= 0 && option <= 6) {
        gSim.jumpToStage(static_cast<Stage>(option));
    } else if (option == 10) gSim.applyCameraPreset(0);
    else if (option == 11) gSim.applyCameraPreset(1);
    else if (option == 12) gSim.applyCameraPreset(2);
    else if (option == 99) gSim.reset();
}

void createContextMenu() {
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Title & Briefing Screen", 0);
    glutAddMenuEntry("Jump -> Stage 1: Forest Trail & Cobra", 1);
    glutAddMenuEntry("Jump -> Stage 2: First Aid & Splint", 2);
    glutAddMenuEntry("Jump -> Stage 3: Stretcher Evac", 3);
    glutAddMenuEntry("Jump -> Stage 4: Highway Ambulance Dodge", 4);
    glutAddMenuEntry("Jump -> Stage 5: Hospital ER (ASV IV)", 5);
    glutAddMenuEntry("Jump -> Stage 6: Clinical Debrief & Rank", 6);
    glutAddMenuEntry("------------------------------", -1);
    glutAddMenuEntry("Camera: Overview", 10);
    glutAddMenuEntry("Camera: Close-Up", 11);
    glutAddMenuEntry("Camera: Side Action", 12);
    glutAddMenuEntry("Restart [R]", 99);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(gWinW, gWinH);
    glutInitWindowPosition(90, 45);
    glutCreateWindow("3D Interactive Snakebite Emergency & First-Aid Simulator (OpenGL)");

    // Initialize audio
    AudioSynth::initAudio();

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

    createContextMenu();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutMouseWheelFunc(mouseWheelCallback);

    gLastFrameMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();
    return 0;
}
