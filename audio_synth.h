#ifndef AUDIO_SYNTH_H
#define AUDIO_SYNTH_H

#include <windows.h>
#include <mmsystem.h>
#include <cmath>
#include <cstring>
#include <vector>

namespace AudioSynth {

static bool gMuted = false;
static float soundCooldown = 0.0f;

inline void update(float dt) {
    if (soundCooldown > 0.0f) {
        soundCooldown -= dt;
        if (soundCooldown < 0.0f) soundCooldown = 0.0f;
    }
}

struct WavBuffer {
    std::vector<unsigned char> data;
};

inline void writeU16LE(std::vector<unsigned char>& v, unsigned short val) {
    v.push_back(val & 0xFF);
    v.push_back((val >> 8) & 0xFF);
}
inline void writeU32LE(std::vector<unsigned char>& v, unsigned int val) {
    v.push_back(val & 0xFF);
    v.push_back((val >> 8) & 0xFF);
    v.push_back((val >> 16) & 0xFF);
    v.push_back((val >> 24) & 0xFF);
}

inline WavBuffer generateWav(int sampleRate, int numSamples, const short* pcm) {
    WavBuffer wb;
    auto& d = wb.data;
    int dataSize = numSamples * 2;
    d.reserve(44 + dataSize);
    // RIFF header
    d.push_back('R'); d.push_back('I'); d.push_back('F'); d.push_back('F');
    writeU32LE(d, 36 + dataSize);
    d.push_back('W'); d.push_back('A'); d.push_back('V'); d.push_back('E');
    // fmt chunk
    d.push_back('f'); d.push_back('m'); d.push_back('t'); d.push_back(' ');
    writeU32LE(d, 16);
    writeU16LE(d, 1); // PCM
    writeU16LE(d, 1); // mono
    writeU32LE(d, sampleRate);
    writeU32LE(d, sampleRate * 2); // byte rate
    writeU16LE(d, 2); // block align
    writeU16LE(d, 16); // bits per sample
    // data chunk
    d.push_back('d'); d.push_back('a'); d.push_back('t'); d.push_back('a');
    writeU32LE(d, dataSize);
    for (int i = 0; i < numSamples; i++) {
        writeU16LE(d, static_cast<unsigned short>(pcm[i]));
    }
    return wb;
}

// Synthesize a tone burst with envelope
inline WavBuffer synthTone(float freqHz, float durationSec, float volume = 0.5f,
                           float attack = 0.01f, float decay = 0.05f) {
    const int sr = 22050;
    int n = static_cast<int>(sr * durationSec);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = 1.0f;
        if (t < attack) env = t / attack;
        if (t > durationSec - decay) env = (durationSec - t) / decay;
        if (env < 0.0f) env = 0.0f;
        float val = std::sin(2.0f * 3.14159265f * freqHz * t) * env * volume;
        pcm[i] = static_cast<short>(val * 28000);
    }
    return generateWav(sr, n, pcm.data());
}

// ECG beep: short high-pitched pip
inline WavBuffer synthECGBeep() {
    return synthTone(1046.5f, 0.08f, 0.35f, 0.005f, 0.03f);
}

// Strike warning: descending dissonant sting
inline WavBuffer synthStrikeWarn() {
    const int sr = 22050;
    int n = static_cast<int>(sr * 0.45f);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = 1.0f - t / 0.45f;
        env *= env;
        float f1 = 880.0f - t * 600.0f;
        float f2 = 1320.0f - t * 900.0f;
        float val = (std::sin(2.0f * 3.14159265f * f1 * t) * 0.5f +
                     std::sin(2.0f * 3.14159265f * f2 * t) * 0.3f) * env * 0.6f;
        pcm[i] = static_cast<short>(val * 26000);
    }
    return generateWav(sr, n, pcm.data());
}

// Positive chime: two ascending notes
inline WavBuffer synthPositiveChime() {
    const int sr = 22050;
    float dur = 0.3f;
    int n = static_cast<int>(sr * dur);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = 1.0f;
        if (t < 0.01f) env = t / 0.01f;
        if (t > dur - 0.08f) env = (dur - t) / 0.08f;
        if (env < 0.0f) env = 0.0f;
        float freq = (t < 0.15f) ? 523.25f : 659.25f;
        float val = std::sin(2.0f * 3.14159265f * freq * t) * env * 0.45f;
        val += std::sin(2.0f * 3.14159265f * freq * 2.0f * t) * env * 0.15f;
        pcm[i] = static_cast<short>(val * 24000);
    }
    return generateWav(sr, n, pcm.data());
}

// Error buzzer: harsh low buzz
inline WavBuffer synthErrorBuzz() {
    const int sr = 22050;
    float dur = 0.35f;
    int n = static_cast<int>(sr * dur);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = 1.0f;
        if (t < 0.01f) env = t / 0.01f;
        if (t > dur - 0.05f) env = (dur - t) / 0.05f;
        if (env < 0.0f) env = 0.0f;
        float ph = 2.0f * 3.14159265f * 185.0f * t;
        // Square-ish wave for harshness
        float val = (std::sin(ph) > 0 ? 1.0f : -1.0f) * 0.3f +
                    std::sin(ph * 3.0f) * 0.15f;
        val *= env;
        pcm[i] = static_cast<short>(val * 22000);
    }
    return generateWav(sr, n, pcm.data());
}

// Siren pulse: wah-wah oscillation
inline WavBuffer synthSirenPulse() {
    const int sr = 22050;
    float dur = 0.6f;
    int n = static_cast<int>(sr * dur);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = 1.0f;
        if (t < 0.02f) env = t / 0.02f;
        if (t > dur - 0.1f) env = (dur - t) / 0.1f;
        if (env < 0.0f) env = 0.0f;
        float freq = 620.0f + 280.0f * std::sin(2.0f * 3.14159265f * 4.5f * t);
        float val = std::sin(2.0f * 3.14159265f * freq * t) * env * 0.4f;
        pcm[i] = static_cast<short>(val * 24000);
    }
    return generateWav(sr, n, pcm.data());
}

// Impact thud
inline WavBuffer synthImpactThud() {
    const int sr = 22050;
    float dur = 0.2f;
    int n = static_cast<int>(sr * dur);
    std::vector<short> pcm(n);
    for (int i = 0; i < n; i++) {
        float t = static_cast<float>(i) / sr;
        float env = std::exp(-t * 18.0f);
        float freq = 120.0f - t * 200.0f;
        if (freq < 40.0f) freq = 40.0f;
        float val = std::sin(2.0f * 3.14159265f * freq * t) * env * 0.7f;
        pcm[i] = static_cast<short>(val * 26000);
    }
    return generateWav(sr, n, pcm.data());
}

// Pre-generated sound buffers
static WavBuffer sndECGBeep;
static WavBuffer sndStrikeWarn;
static WavBuffer sndPositiveChime;
static WavBuffer sndErrorBuzz;
static WavBuffer sndSirenPulse;
static WavBuffer sndImpactThud;
static bool sInitialized = false;

inline void initAudio() {
    if (sInitialized) return;
    sndECGBeep = synthECGBeep();
    sndStrikeWarn = synthStrikeWarn();
    sndPositiveChime = synthPositiveChime();
    sndErrorBuzz = synthErrorBuzz();
    sndSirenPulse = synthSirenPulse();
    sndImpactThud = synthImpactThud();
    sInitialized = true;
}

inline void playSound(const WavBuffer& wb) {
    if (gMuted || wb.data.empty()) return;
    PlaySoundA(reinterpret_cast<LPCSTR>(wb.data.data()),
               NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

inline void playECGBeep() {
    if (soundCooldown <= 0.0f) {
        playSound(sndECGBeep);
    }
}
inline void playStrikeWarn()    { soundCooldown = 1.2f; playSound(sndStrikeWarn); }
inline void playPositiveChime() { soundCooldown = 1.2f; playSound(sndPositiveChime); }
inline void playErrorBuzz()     { soundCooldown = 1.2f; playSound(sndErrorBuzz); }
inline void playSirenPulse()    { soundCooldown = 1.2f; playSound(sndSirenPulse); }
inline void playImpactThud()    { soundCooldown = 1.2f; playSound(sndImpactThud); }

inline void toggleMute() { gMuted = !gMuted; }
inline bool isMuted() { return gMuted; }

} // namespace AudioSynth

#endif // AUDIO_SYNTH_H
