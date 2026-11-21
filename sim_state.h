#ifndef SIM_STATE_H
#define SIM_STATE_H

#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

enum Stage {
    STAGE_TITLE = 0,
    STAGE_RECOGNITION = 1,
    STAGE_FIRST_AID = 2,
    STAGE_HANDLING = 3,
    STAGE_TRANSPORT = 4,
    STAGE_HOSPITAL = 5,
    STAGE_DEBRIEF = 6
};

struct DecisionRecord {
    int stage;
    std::string action;
    bool isCorrect;
    std::string clinicalNote;
};

inline float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * clampf(t, 0.0f, 1.0f);
}

// ============================================================
// 3D PARTICLE SYSTEM
// ============================================================
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life, maxLife;
    float r, g, b, a;
    float size;
};

static const int MAX_PARTICLES = 200;

struct ParticleSystem {
    Particle particles[MAX_PARTICLES];
    int count;

    ParticleSystem() : count(0) {}

    void clear() { count = 0; }

    void emit(float x, float y, float z,
              float vx, float vy, float vz,
              float life, float r, float g, float b, float size = 0.08f) {
        if (count >= MAX_PARTICLES) return;
        Particle& p = particles[count++];
        p.x = x; p.y = y; p.z = z;
        p.vx = vx; p.vy = vy; p.vz = vz;
        p.life = life; p.maxLife = life;
        p.r = r; p.g = g; p.b = b; p.a = 1.0f;
        p.size = size;
    }

    void update(float dt) {
        for (int i = 0; i < count; ) {
            Particle& p = particles[i];
            p.life -= dt;
            if (p.life <= 0.0f) {
                particles[i] = particles[--count];
                continue;
            }
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;
            p.vy -= 1.2f * dt; // gravity
            p.a = clampf(p.life / p.maxLife, 0.0f, 1.0f);
            ++i;
        }
    }
};

// ============================================================
// SIM STATE - FULL GAMEPLAY
// ============================================================
struct SimState {
    // Core stage & timing
    Stage currentStage;
    float simTime;
    float stageTime;
    float elapsedTime;

    // Patient vitals & clinical state
    float venomLevel;
    float heartRate;
    float targetHeartRate;
    float stabilityScore;
    bool patientSurvived;
    bool emergencyFinished;

    // Stage-specific visual & gameplay states
    bool isVenomous;
    bool strikeComplete;
    bool recognitionDone;
    bool limbImmobilized;
    bool patientCalmed;
    bool onStretcher;
    float stretcherLiftProgress;
    bool sirenActive;
    bool vitalsMonitored;
    float distanceToHospital;
    float roadScrollOffset;
    bool asvAdministered;
    float ivFluidLevel;
    float ecgPhase;

    // ======= PLAYABLE GAMEPLAY ADDITIONS =======
    // Player position (WASD movement)
    float playerX, playerZ;
    float playerAngle;     // facing direction (degrees)
    float playerSpeed;     // current movement speed
    float playerWalkPhase; // animation phase
    bool playerMoving;

    // Stage 1: Hiker approach
    bool hikerApproached;  // close enough to trigger snake
    float snakeX, snakeZ;  // snake world position (moves toward hiker)
    float snakeTargetYaw;  // facing direction
    bool snakeApproaching; // snake is slithering toward player

    // Stage 2: Splint tension meter mini-game
    bool splintMiniGameActive;
    float tensionNeedle;       // 0.0 to 1.0, oscillating
    float tensionNeedleSpeed;
    bool splintSuccess;        // green zone hit

    // Stage 3: Stretcher carry
    float stretcherCarryX, stretcherCarryZ;
    float stretcherBPMPenalty; // accumulated from rough movement
    bool reachedExtractionZone;

    // Stage 4: Ambulance dodge
    int ambulanceLane;      // 0=left, 1=center, 2=right
    float ambulanceLaneX;   // smooth interpolation target
    float ambulanceActualX; // smoothed position
    int dodgeCombo;
    int maxDodgeCombo;
    float dodgeCooldown;    // timer between lane changes

    // Obstacle cars
    struct ObstacleCar {
        float x, z;
        int lane;
        float speed;
        float r, g, b;
        bool active;
    };
    static const int MAX_OBSTACLES = 8;
    ObstacleCar obstacles[MAX_OBSTACLES];
    float obstacleSpawnTimer;
    int obstaclesAvoided;

    // Stage 5: IV drip flow rate
    float ivFlowRate;      // 0.0 to 1.0, player controls
    float ivOptimalLow;    // green zone bounds
    float ivOptimalHigh;

    // Camera system
    float camYaw, camPitch, camDist;
    int camPreset;
    float camTargetYaw, camTargetPitch, camTargetDist;
    float camShakeIntensity;
    float camShakeTimer;
    float camOffsetX, camOffsetY; // shake offsets

    // Screen effects
    float vignetteFlash;      // red vignette intensity 0..1
    float vignetteTimer;

    // Scoring
    int score;
    int dodgeScore;
    float timeBonus;
    bool zeroMistakes;
    std::string clinicalRank;

    // ECG beep timer
    float ecgBeepTimer;
    float lastEcgBeepInterval;

    // Siren sound timer
    float sirenSoundTimer;

    // Feedback toast
    std::string feedbackTitle;
    std::string feedbackMsg;
    int feedbackType;
    float feedbackTimer;

    // Decision log
    std::vector<DecisionRecord> decisionLog;
    int correctDecisions;
    int wrongDecisions;

    // Particles
    ParticleSystem particles;

    // Foot dust timer
    float dustTimer;

    SimState() { reset(); }

    void reset() {
        currentStage = STAGE_TITLE;
        simTime = 0.0f;
        stageTime = 0.0f;
        elapsedTime = 0.0f;

        venomLevel = 8.0f;
        heartRate = 82.0f;
        targetHeartRate = 82.0f;
        stabilityScore = 100.0f;
        patientSurvived = true;
        emergencyFinished = false;

        isVenomous = true;
        strikeComplete = false;
        recognitionDone = false;
        limbImmobilized = false;
        patientCalmed = false;
        onStretcher = false;
        stretcherLiftProgress = 0.0f;
        sirenActive = false;
        vitalsMonitored = false;
        distanceToHospital = 16.0f;
        roadScrollOffset = 0.0f;
        asvAdministered = false;
        ivFluidLevel = 1.0f;
        ecgPhase = 0.0f;

        // Player
        playerX = 0.0f; playerZ = 8.0f;
        playerAngle = 0.0f;
        playerSpeed = 0.0f;
        playerWalkPhase = 0.0f;
        playerMoving = false;

        hikerApproached = false;
        snakeX = 3.5f; snakeZ = -2.2f; // start near trailside rocks
        snakeTargetYaw = 0.0f;
        snakeApproaching = false;

        // Splint mini-game
        splintMiniGameActive = false;
        tensionNeedle = 0.0f;
        tensionNeedleSpeed = 2.2f;
        splintSuccess = false;

        // Stretcher carry
        stretcherCarryX = 0.0f;
        stretcherCarryZ = 0.0f;
        stretcherBPMPenalty = 0.0f;
        reachedExtractionZone = false;

        // Ambulance dodge
        ambulanceLane = 1;
        ambulanceLaneX = 0.0f;
        ambulanceActualX = 0.0f;
        dodgeCombo = 0;
        maxDodgeCombo = 0;
        dodgeCooldown = 0.0f;
        for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
        obstacleSpawnTimer = 1.5f;
        obstaclesAvoided = 0;

        // IV drip
        ivFlowRate = 0.5f;
        ivOptimalLow = 0.35f;
        ivOptimalHigh = 0.65f;

        // Camera
        camYaw = 32.0f; camPitch = 22.0f; camDist = 11.5f;
        camPreset = 0;
        camTargetYaw = 32.0f; camTargetPitch = 22.0f; camTargetDist = 11.5f;
        camShakeIntensity = 0.0f;
        camShakeTimer = 0.0f;
        camOffsetX = 0.0f; camOffsetY = 0.0f;

        // Effects
        vignetteFlash = 0.0f;
        vignetteTimer = 0.0f;

        // Scoring
        score = 0;
        dodgeScore = 0;
        timeBonus = 0.0f;
        zeroMistakes = true;
        clinicalRank = "F";

        // ECG beep
        ecgBeepTimer = 0.0f;
        lastEcgBeepInterval = 0.75f;

        sirenSoundTimer = 0.0f;

        feedbackTitle = "READY";
        feedbackMsg = "Press ENTER to begin the 3D Snakebite Emergency Simulation.";
        feedbackType = 0;
        feedbackTimer = 999.0f;

        decisionLog.clear();
        correctDecisions = 0;
        wrongDecisions = 0;

        particles.clear();
        dustTimer = 0.0f;
    }

    void setFeedback(const std::string& title, const std::string& msg, int type, float duration = 5.5f) {
        feedbackTitle = title;
        feedbackMsg = msg;
        feedbackType = type;
        feedbackTimer = duration;
    }

    void recordDecision(const std::string& action, bool correct, const std::string& note) {
        DecisionRecord rec;
        rec.stage = static_cast<int>(currentStage);
        rec.action = action;
        rec.isCorrect = correct;
        rec.clinicalNote = note;
        decisionLog.push_back(rec);

        if (correct) {
            correctDecisions++;
            stabilityScore = clampf(stabilityScore + 4.0f, 0.0f, 100.0f);
            score += 12;
        } else {
            wrongDecisions++;
            zeroMistakes = false;
            stabilityScore = clampf(stabilityScore - 16.0f, 0.0f, 100.0f);
        }
    }

    void triggerCameraShake(float intensity, float duration) {
        camShakeIntensity = intensity;
        camShakeTimer = duration;
    }

    void triggerVignette(float intensity, float duration) {
        vignetteFlash = intensity;
        vignetteTimer = duration;
    }

    void applyCameraPreset(int preset) {
        camPreset = preset % 3;
        if (camPreset == 0) {
            camTargetYaw = 34.0f;
            camTargetPitch = 24.0f;
            if (currentStage == STAGE_TRANSPORT) {
                // Clean behind-the-ambulance chase view looking forward down highway
                camTargetYaw = 0.0f;
                camTargetPitch = 20.0f;
                camTargetDist = 11.5f;
            } else {
                camTargetDist = 11.5f;
            }
        } else if (camPreset == 1) {
            camTargetYaw = 15.0f;
            camTargetPitch = 16.0f;
            camTargetDist = 6.2f;
        } else {
            camTargetYaw = 88.0f;
            camTargetPitch = 18.0f;
            camTargetDist = 11.0f;
        }
    }

    void jumpToStage(Stage target) {
        currentStage = target;
        stageTime = 0.0f;
        particles.clear();

        if (target == STAGE_TITLE) { reset(); return; }

        if (target >= STAGE_FIRST_AID) {
            strikeComplete = true;
            recognitionDone = true;
        }
        if (target >= STAGE_HANDLING) {
            limbImmobilized = true;
            patientCalmed = true;
            splintSuccess = true;
        }
        if (target >= STAGE_TRANSPORT) {
            onStretcher = true;
            stretcherLiftProgress = 1.0f;
            distanceToHospital = 16.0f;
            reachedExtractionZone = true;
            ambulanceLane = 1;
            ambulanceLaneX = 0.0f;
            ambulanceActualX = 0.0f;
            for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
        }
        if (target == STAGE_HOSPITAL) {
            distanceToHospital = 0.0f;
            asvAdministered = false;
            ivFluidLevel = 1.0f;
            ivFlowRate = 0.5f;
            venomLevel = clampf(venomLevel, 30.0f, 75.0f);
        }
        if (target == STAGE_RECOGNITION) {
            playerX = 0.0f; playerZ = 8.0f;
            strikeComplete = false;
            hikerApproached = false;
            snakeX = 3.5f; snakeZ = -2.2f;
            snakeTargetYaw = 0.0f;
            snakeApproaching = false;
        }
        if (target == STAGE_FIRST_AID) {
            playerX = 2.0f; playerZ = 0.5f;
            splintMiniGameActive = false;
            splintSuccess = false;
        }
        if (target == STAGE_HANDLING) {
            playerX = 0.0f; playerZ = 0.0f;
            stretcherCarryX = 0.0f; stretcherCarryZ = 0.0f;
            reachedExtractionZone = false;
        }

        switch (target) {
            case STAGE_RECOGNITION:
                applyCameraPreset(0);
                setFeedback("STAGE 1: FOREST TRAIL", "Walk forward with WASD toward the trail bend. Watch for the cobra!", 0, 8.0f);
                break;
            case STAGE_FIRST_AID:
                applyCameraPreset(1);
                setFeedback("STAGE 2: FIRST AID", "Move to the patient. Press [1] for splint (hit SPACE in green zone!). Avoid [2] [3] myths!", 0, 8.0f);
                break;
            case STAGE_HANDLING:
                applyCameraPreset(0);
                setFeedback("STAGE 3: STRETCHER EVAC", "Press [1] to load patient. Then carry stretcher with WASD to the green zone!", 0, 8.0f);
                break;
            case STAGE_TRANSPORT:
                applyCameraPreset(2);
                
                setFeedback("STAGE 4: HIGHWAY DODGE!", "Steer A/D between 3 lanes! Dodge cars! [1] Siren boost, [2] Monitor vitals.", 0, 8.0f);
                break;
            case STAGE_HOSPITAL:
                applyCameraPreset(0);
                setFeedback("STAGE 5: ER TREATMENT", "Press [1] ASV IV. Then regulate drip with W/S to keep flow in GREEN zone!", 0, 8.0f);
                break;
            case STAGE_DEBRIEF:
                emergencyFinished = true;
                computeRank();
                applyCameraPreset(0);
                setFeedback("SIMULATION COMPLETE", "Review your clinical performance. Press R to restart.", 1, 999.0f);
                break;
            default: break;
        }
    }

    // Returns true if an ECG beep should fire this frame
    bool shouldBeepECG(float dt) {
        float interval = 60.0f / heartRate;
        ecgBeepTimer += dt;
        if (ecgBeepTimer >= interval) {
            ecgBeepTimer -= interval;
            return true;
        }
        return false;
    }

    void update(float dt) {
        simTime += dt;
        stageTime += dt;
        ecgPhase += dt * (heartRate / 60.0f) * 2.2f;

        if (feedbackTimer > 0.0f && feedbackTimer < 500.0f) feedbackTimer -= dt;

        // Smooth heart rate
        heartRate += (targetHeartRate - heartRate) * clampf(dt * 2.5f, 0.0f, 1.0f);

        // Camera shake decay
        if (camShakeTimer > 0.0f) {
            camShakeTimer -= dt;
            float shakeMag = camShakeIntensity * clampf(camShakeTimer / 0.4f, 0.0f, 1.0f);
            camOffsetX = std::sin(simTime * 47.0f) * shakeMag;
            camOffsetY = std::cos(simTime * 53.0f) * shakeMag * 0.7f;
        } else {
            camOffsetX *= 0.85f;
            camOffsetY *= 0.85f;
        }

        // Smooth camera
        camYaw += (camTargetYaw - camYaw) * clampf(dt * 4.0f, 0.0f, 1.0f);
        camPitch += (camTargetPitch - camPitch) * clampf(dt * 4.0f, 0.0f, 1.0f);
        camDist += (camTargetDist - camDist) * clampf(dt * 4.0f, 0.0f, 1.0f);

        // Vignette decay
        if (vignetteTimer > 0.0f) {
            vignetteTimer -= dt;
        } else {
            vignetteFlash *= 0.92f;
        }

        // Critical venom vignette
        if (venomLevel > 70.0f && currentStage != STAGE_DEBRIEF) {
            float pulse = 0.15f + 0.1f * std::sin(simTime * 5.0f);
            vignetteFlash = std::max(vignetteFlash, pulse * (venomLevel / 100.0f));
        }

        // Update particles
        particles.update(dt);

        // Walk phase animation
        if (playerMoving && playerSpeed > 0.1f) {
            playerWalkPhase += dt * playerSpeed * 4.5f;
            dustTimer += dt;
            if (dustTimer > 0.25f) {
                dustTimer = 0.0f;
                // emit dust
                float rx = (std::sin(simTime * 123.456f) * 0.5f);
                float rz = (std::cos(simTime * 789.012f) * 0.5f);
                particles.emit(playerX + rx * 0.3f, 0.05f, playerZ + rz * 0.3f,
                              rx * 0.4f, 0.6f, rz * 0.4f,
                              0.6f, 0.6f, 0.5f, 0.4f, 0.1f);
            }
        }

        if (currentStage == STAGE_TITLE || currentStage == STAGE_DEBRIEF) {
            // Title screen: emit slow drifting ambient dust/glow particles
            if (currentStage == STAGE_TITLE) {
                dustTimer += dt;
                if (dustTimer > 0.35f) {
                    dustTimer = 0.0f;
                    float px = std::sin(simTime * 1.3f) * 4.0f;
                    float pz = std::cos(simTime * 0.9f) * 4.0f;
                    particles.emit(px, 0.5f + std::sin(simTime * 2.1f) * 0.3f, pz,
                                  std::sin(simTime * 0.7f) * 0.3f, 0.4f, std::cos(simTime * 1.1f) * 0.3f,
                                  2.5f, 0.15f, 0.55f, 0.75f, 0.04f);
                }
                particles.update(dt);
            }
            return;
        }

        elapsedTime += dt;

        // ===== STAGE 1: Recognition — Snake AI with proximity bite =====
        if (currentStage == STAGE_RECOGNITION) {
            if (!strikeComplete) {
                // Calculate distance from player to snake
                float dx = playerX - snakeX;
                float dz = playerZ - snakeZ;
                float dist = std::sqrt(dx*dx + dz*dz);

                // Snake begins approaching when player is within 6 units
                if (dist < 6.0f && !snakeApproaching) {
                    snakeApproaching = true;
                }

                // Also trigger approach if player presses E (hikerApproached)
                if (hikerApproached && !snakeApproaching) {
                    snakeApproaching = true;
                }

                // Snake slithers toward player
                if (snakeApproaching && dist > 1.2f) {
                    float snakeSpeed = 1.8f; // units/sec
                    float invDist = 1.0f / (dist + 0.001f);
                    snakeX += dx * invDist * snakeSpeed * dt;
                    snakeZ += dz * invDist * snakeSpeed * dt;
                    // Face toward player
                    snakeTargetYaw = std::atan2(dx, dz) * (180.0f / 3.14159265f);
                }

                // Strike when snake closes to within 1.4 units of player's right ankle
                if (snakeApproaching && dist < 1.4f && !strikeComplete) {
                    strikeComplete = true;
                    hikerApproached = true;
                    targetHeartRate = 114.0f;
                    venomLevel = 16.0f;
                    triggerCameraShake(0.5f, 0.6f);
                    triggerVignette(0.6f, 0.5f);
                    setFeedback("SNAKEBITE!", "Two fang punctures on right ankle! Identify species & protocol: [1] Venomous [2] Non-Venomous [3] Chase", 2, 8.0f);
                    AudioSynth::playStrikeWarn();
                    // After strike, coil defensively nearby
                    snakeApproaching = false;
                }
            }
        }

        // ===== STAGE 2: Splint mini-game =====
        if (currentStage == STAGE_FIRST_AID && splintMiniGameActive && !splintSuccess) {
            tensionNeedle += tensionNeedleSpeed * dt;
            if (tensionNeedle > 1.0f || tensionNeedle < 0.0f) {
                tensionNeedleSpeed = -tensionNeedleSpeed;
                tensionNeedle = clampf(tensionNeedle, 0.0f, 1.0f);
            }
            // Speed up over time for challenge
            float absSpeed = std::abs(tensionNeedleSpeed);
            absSpeed += dt * 0.3f;
            tensionNeedleSpeed = (tensionNeedleSpeed > 0 ? absSpeed : -absSpeed);
        }

        // ===== STAGE 3: Stretcher evac =====
        if (currentStage == STAGE_HANDLING && onStretcher && stretcherLiftProgress >= 1.0f) {
            // Check if reached extraction zone (z < -12)
            if (stretcherCarryZ < -12.0f && !reachedExtractionZone) {
                reachedExtractionZone = true;
                score += 15;
                setFeedback("EXTRACTION ZONE!", "Patient loaded into ambulance. Departing for ER!", 1, 4.0f);
                jumpToStage(STAGE_TRANSPORT);
                return;
            }
        }

        // Smooth stretcher lift
        if (currentStage == STAGE_HANDLING && onStretcher && stretcherLiftProgress < 1.0f) {
            stretcherLiftProgress = clampf(stretcherLiftProgress + dt * 0.55f, 0.0f, 1.0f);
        }

        // ===== STAGE 4: Ambulance dodge =====
        if (currentStage == STAGE_TRANSPORT) {
            float speedKmPerSec = sirenActive ? 0.62f : 0.38f;
            distanceToHospital = clampf(distanceToHospital - speedKmPerSec * dt, 0.0f, 16.0f);
            roadScrollOffset += (sirenActive ? 18.0f : 10.0f) * dt;
            if (roadScrollOffset > 20.0f) roadScrollOffset -= 20.0f;

            if (dodgeCooldown > 0.0f) dodgeCooldown -= dt;

            // Smooth lane interpolation
            float targetLaneX = (ambulanceLane - 1) * 2.5f;
            ambulanceLaneX = targetLaneX;
            ambulanceActualX += (ambulanceLaneX - ambulanceActualX) * clampf(dt * 8.0f, 0.0f, 1.0f);

            // Spawn obstacles
            obstacleSpawnTimer -= dt;
            if (obstacleSpawnTimer <= 0.0f) {
                obstacleSpawnTimer = 1.0f + std::sin(simTime * 0.7f) * 0.4f;
                if (sirenActive) obstacleSpawnTimer *= 0.65f;

                for (int i = 0; i < MAX_OBSTACLES; i++) {
                    if (!obstacles[i].active) {
                        obstacles[i].active = true;
                        obstacles[i].lane = static_cast<int>(std::abs(std::sin(simTime * 3.7f + i)) * 2.99f);
                        obstacles[i].x = (obstacles[i].lane - 1) * 2.5f;
                        obstacles[i].z = -45.0f;
                        obstacles[i].speed = sirenActive ? 14.0f : 8.0f;
                        // Random-ish color
                        float h = std::fmod(simTime * 0.3f + i * 1.1f, 3.0f);
                        obstacles[i].r = (h < 1.0f) ? 0.8f : 0.3f;
                        obstacles[i].g = (h >= 1.0f && h < 2.0f) ? 0.7f : 0.3f;
                        obstacles[i].b = (h >= 2.0f) ? 0.8f : 0.4f;
                        break;
                    }
                }
            }

            // Update obstacles
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (!obstacles[i].active) continue;
                obstacles[i].z += obstacles[i].speed * dt;

                // Collision check (obstacle passes ambulance position at z ~= 0)
                if (obstacles[i].z > -1.5f && obstacles[i].z < 2.0f) {
                    float dx = std::abs(obstacles[i].x - ambulanceActualX);
                    if (dx < 1.8f) {
                        // HIT!
                        obstacles[i].active = false;
                        AudioSynth::playImpactThud();
                        triggerCameraShake(0.7f, 0.5f);
                        triggerVignette(0.7f, 0.4f);
                        venomLevel = clampf(venomLevel + 3.0f, 0.0f, 100.0f);
                        targetHeartRate = clampf(targetHeartRate + 8.0f, 60.0f, 175.0f);
                        dodgeCombo = 0;
                        score = std::max(0, score - 5);
                        setFeedback("COLLISION!", "Impact! Patient distressed. BPM spiking! Keep steady!", 2, 2.5f);
                        // Spawn impact particles
                        for (int p = 0; p < 8; p++) {
                            float angle = p * 0.785f;
                            particles.emit(ambulanceActualX, 1.0f, 0.0f,
                                          std::cos(angle) * 3.0f, 2.0f + std::sin(angle), std::sin(angle) * 2.0f,
                                          0.8f, 1.0f, 0.4f, 0.1f, 0.12f);
                        }
                        continue;
                    }
                }

                // Passed safely
                if (obstacles[i].z > 5.0f) {
                    obstacles[i].active = false;
                    dodgeCombo++;
                    if (dodgeCombo > maxDodgeCombo) maxDodgeCombo = dodgeCombo;
                    obstaclesAvoided++;
                    dodgeScore += 2 + dodgeCombo;
                    score += 2;
                }
            }

            if (distanceToHospital <= 0.0f) {
                jumpToStage(STAGE_HOSPITAL);
                return;
            }
        }

        // ===== STAGE 5: Hospital IV Drip =====
        if (currentStage == STAGE_HOSPITAL && asvAdministered) {
            // Player-controlled IV flow rate affects venom drain speed
            bool inZone = (ivFlowRate >= ivOptimalLow && ivFlowRate <= ivOptimalHigh);
            float drainMult = inZone ? 1.4f : 0.6f;

            if (ivFlowRate > ivOptimalHigh + 0.15f) {
                // Too fast: anaphylaxis risk warning
                targetHeartRate = clampf(targetHeartRate + dt * 8.0f, 60.0f, 165.0f);
            } else if (inZone) {
                targetHeartRate = clampf(targetHeartRate - dt * 3.0f, 68.0f, 160.0f);
                // Healing particles
                if (std::fmod(simTime, 0.4f) < dt) {
                    particles.emit(1.55f, 1.5f, -0.75f,
                                  (std::sin(simTime * 7.0f)) * 0.5f, 1.5f, (std::cos(simTime * 11.0f)) * 0.5f,
                                  1.2f, 0.2f, 1.0f, 0.6f, 0.06f);
                }
            }

            ivFluidLevel = clampf(ivFluidLevel - dt * 0.075f * drainMult, 0.0f, 1.0f);
            venomLevel = clampf(venomLevel - dt * 7.5f * drainMult, 0.0f, 100.0f);

            if (venomLevel <= 0.5f || ivFluidLevel <= 0.05f) {
                venomLevel = 0.0f;
                patientSurvived = true;
                score += 20;
                jumpToStage(STAGE_DEBRIEF);
                return;
            }
        } else if (strikeComplete && currentStage != STAGE_HOSPITAL) {
            float baseRate = 0.85f;
            if (limbImmobilized) baseRate *= 0.48f;
            if (patientCalmed) baseRate *= 0.80f;
            float hrMultiplier = 0.65f + (heartRate - 70.0f) / 140.0f;
            venomLevel = clampf(venomLevel + baseRate * hrMultiplier * dt, 0.0f, 100.0f);
        }

        // Critical failure
        if (venomLevel >= 100.0f) {
            patientSurvived = false;
            stabilityScore = clampf(stabilityScore - 35.0f, 0.0f, 100.0f);
            jumpToStage(STAGE_DEBRIEF);
        }
    }

    void handleActionKey(int choice) {
        if (currentStage == STAGE_RECOGNITION) {
            if (!strikeComplete) return; // must wait for strike
            if (choice == 1) {
                recognitionDone = true;
                recordDecision("Identified Venomous Cobra & Initiated High-Alert Protocol", true,
                               "Flared hood and paired fang punctures indicate venomous elapid bite.");
                setFeedback("CORRECT!", "Venomous Indian Cobra recognized! Moving to First Aid...", 1, 4.5f);
                jumpToStage(STAGE_FIRST_AID);
            } else if (choice == 2) {
                venomLevel = clampf(venomLevel + 9.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 10.0f, 60.0f, 175.0f);
                triggerCameraShake(0.3f, 0.3f);
                triggerVignette(0.5f, 0.4f);
                recordDecision("Misidentified as Non-Venomous", false,
                               "Delaying care for a venomous hood-flaring cobra worsens systemic envenomation.");
                setFeedback("WRONG!", "Notice the flared hood & dual fang marks. Always treat as venomous!", 2, 5.5f);
            } else if (choice == 3) {
                venomLevel = clampf(venomLevel + 12.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 22.0f, 60.0f, 175.0f);
                triggerCameraShake(0.4f, 0.4f);
                triggerVignette(0.6f, 0.5f);
                recordDecision("Attempted to Chase/Kill Snake", false,
                               "Chasing the snake wastes critical minutes and risks a secondary bite.");
                setFeedback("DANGEROUS!", "Never chase the snake! Move away and focus on the victim.", 2, 5.5f);
            }
        }
        else if (currentStage == STAGE_FIRST_AID) {
            if (choice == 1) {
                if (!limbImmobilized && !splintMiniGameActive) {
                    splintMiniGameActive = true;
                    tensionNeedle = 0.0f;
                    tensionNeedleSpeed = 2.2f;
                    setFeedback("SPLINT MINI-GAME!", "Press SPACE when the needle is in the GREEN SAFE ZONE!", 0, 10.0f);
                } else if (limbImmobilized) {
                    setFeedback("ALREADY APPLIED", "Limb is already splinted. Press [4] to calm patient & call 108.", 0, 4.0f);
                }
            } else if (choice == 2) {
                venomLevel = clampf(venomLevel + 11.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 14.0f, 60.0f, 175.0f);
                triggerCameraShake(0.4f, 0.35f);
                triggerVignette(0.7f, 0.5f);
                recordDecision("Tied Tight Tourniquet Above Bite", false,
                               "Tourniquets trap venom locally causing severe tissue necrosis and amputation risk.");
                setFeedback("NO TOURNIQUET!", "Tight tourniquets cause tissue necrosis. Use a loose splint!", 2, 6.0f);
            } else if (choice == 3) {
                venomLevel = clampf(venomLevel + 14.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 20.0f, 60.0f, 175.0f);
                triggerCameraShake(0.5f, 0.4f);
                triggerVignette(0.8f, 0.5f);
                recordDecision("Cut Wound & Attempted to Suck Venom", false,
                               "Cutting causes hemorrhage/infection and cannot remove venom from deep tissue.");
                setFeedback("DO NOT CUT!", "Cutting or sucking causes bleeding and infection with zero benefit!", 2, 6.0f);
            } else if (choice == 4) {
                if (!patientCalmed) {
                    patientCalmed = true;
                    targetHeartRate = clampf(targetHeartRate - 22.0f, 74.0f, 160.0f);
                    recordDecision("Calmed Patient, Removed Tight Items & Called 108", true,
                                   "Lowering heart rate slows circulation and prevents swelling constriction.");
                    if (limbImmobilized) {
                        setFeedback("STABILIZED!", "108 dispatched. Responder arriving with stretcher...", 1, 4.5f);
                        jumpToStage(STAGE_HANDLING);
                    } else {
                        setFeedback("NOW SPLINT!", "Patient calmed & 108 called. Press [1] to splint the leg!", 1, 5.5f);
                    }
                }
            }
        }
        else if (currentStage == STAGE_HANDLING) {
            if (choice == 1) {
                if (!onStretcher) {
                    onStretcher = true;
                    targetHeartRate = clampf(targetHeartRate - 8.0f, 72.0f, 160.0f);
                    recordDecision("Kept Patient Flat on Stretcher & Stabilized Limb", true,
                                   "Zero physical exertion prevents muscle pumping of venom into bloodstream.");
                    setFeedback("LOADED!", "Carry stretcher with WASD to the GREEN extraction zone!", 1, 5.0f);
                }
            } else if (choice == 2) {
                venomLevel = clampf(venomLevel + 15.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 28.0f, 60.0f, 178.0f);
                triggerCameraShake(0.3f, 0.3f);
                triggerVignette(0.5f, 0.4f);
                recordDecision("Made Patient Walk to Road", false,
                               "Walking elevates heart rate and accelerates systemic venom absorption.");
                setFeedback("DANGEROUS!", "Never make a snakebite victim walk! Press [1] for stretcher!", 2, 5.5f);
            }
        }
        else if (currentStage == STAGE_TRANSPORT) {
            if (choice == 1) {
                if (!sirenActive) {
                    sirenActive = true;
                    recordDecision("Activated Emergency Siren & Priority Lane", true,
                                   "Rapid hospital arrival is the single most decisive factor in snakebite survival.");
                    setFeedback("SIREN ON!", "Speed doubled! Dodge traffic with A/D! Press [2] to monitor vitals.", 1, 5.0f);
                }
            } else if (choice == 2) {
                if (!vitalsMonitored) {
                    vitalsMonitored = true;
                    targetHeartRate = clampf(targetHeartRate - 12.0f, 70.0f, 160.0f);
                    recordDecision("Monitored Airway, Breathing & Vitals En Route", true,
                                   "Continuous airway monitoring catches early neurotoxic respiratory distress.");
                    setFeedback("VITALS OK", "Airway clear, BPM stabilizing. Keep dodging traffic!", 1, 5.0f);
                }
            }
        }
        else if (currentStage == STAGE_HOSPITAL) {
            if (choice == 1) {
                if (!asvAdministered) {
                    asvAdministered = true;
                    recordDecision("Administered Polyvalent Anti-Snake Venom (ASV) IV", true,
                                   "Polyvalent ASV is the only definitive medical cure for venomous snakebite.");
                    setFeedback("ASV STARTED!", "Regulate IV flow with W/S keys. Keep needle in GREEN zone!", 1, 7.0f);
                }
            } else if (choice == 2) {
                venomLevel = clampf(venomLevel + 13.0f, 0.0f, 100.0f);
                targetHeartRate = clampf(targetHeartRate + 12.0f, 60.0f, 175.0f);
                triggerCameraShake(0.3f, 0.3f);
                triggerVignette(0.6f, 0.4f);
                recordDecision("Delayed ASV / Administered Unverified Painkiller", false,
                               "Painkillers do not neutralize venom and NSAIDs can worsen coagulopathy bleeding.");
                setFeedback("WRONG!", "Painkillers won't stop venom! Press [1] for ASV IV now!", 2, 6.0f);
            }
        }
    }

    void handleSplintHit() {
        if (!splintMiniGameActive || splintSuccess) return;
        // Green safe zone: 0.3 to 0.6
        if (tensionNeedle >= 0.3f && tensionNeedle <= 0.6f) {
            splintSuccess = true;
            limbImmobilized = true;
            splintMiniGameActive = false;
            recordDecision("Immobilized Limb with Splint (Precision Applied)", true,
                           "Immobilization slows lymphatic venom spread by nearly 50%.");
            score += 10;
            if (patientCalmed) {
                setFeedback("STABILIZED!", "Splint applied & 108 called. Responder arriving...", 1, 4.5f);
                jumpToStage(STAGE_HANDLING);
            } else {
                setFeedback("PERFECT SPLINT!", "Firm-but-safe wrap applied! Press [4] to calm & call 108.", 1, 6.0f);
            }
        } else if (tensionNeedle > 0.6f) {
            // Too tight - tourniquet warning
            triggerCameraShake(0.3f, 0.3f);
            triggerVignette(0.5f, 0.3f);
            tensionNeedle = 0.5f;
            setFeedback("TOO TIGHT!", "Wrapping too tight risks tourniquet necrosis! Try again in GREEN zone!", 2, 3.5f);
        } else {
            // Too loose
            tensionNeedle = 0.5f;
            setFeedback("TOO LOOSE!", "Splint won't hold. Press SPACE when needle is in the GREEN zone!", 0, 3.0f);
        }
    }

    std::string getStageName() const {
        switch (currentStage) {
            case STAGE_TITLE:       return "BRIEFING";
            case STAGE_RECOGNITION: return "STAGE 1: SNAKEBITE RECOGNITION";
            case STAGE_FIRST_AID:   return "STAGE 2: IMMEDIATE FIRST AID";
            case STAGE_HANDLING:    return "STAGE 3: STRETCHER EVAC";
            case STAGE_TRANSPORT:   return "STAGE 4: HIGHWAY DODGE";
            case STAGE_HOSPITAL:    return "STAGE 5: HOSPITAL ER";
            case STAGE_DEBRIEF:     return "STAGE 6: CLINICAL DEBRIEF";
            default:                return "SIMULATION";
        }
    }

    int computeFinalScore() const {
        float s = stabilityScore;
        s += dodgeScore * 0.5f;
        if (!patientSurvived) s = clampf(s * 0.35f, 0.0f, 40.0f);
        if (elapsedTime < 65.0f && patientSurvived) s += 5.0f;
        if (zeroMistakes && patientSurvived) s += 10.0f;
        s += score * 0.3f;
        return static_cast<int>(clampf(s, 0.0f, 100.0f) + 0.5f);
    }

    void computeRank() {
        int fs = computeFinalScore();
        if (fs >= 95 && zeroMistakes && patientSurvived) clinicalRank = "S";
        else if (fs >= 85 && patientSurvived) clinicalRank = "A";
        else if (fs >= 70 && patientSurvived) clinicalRank = "B";
        else if (fs >= 50) clinicalRank = "C";
        else clinicalRank = "F";
    }
};

#endif // SIM_STATE_H
