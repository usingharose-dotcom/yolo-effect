// MonkEngine.h - framework-independent DSP for "YOLO Monk Machine"
// Monophonic singing-voice synth: band-limited glottal pulse -> 5 formant
// resonators, with growl / jitter / robot / crush character, glide, delayed
// vibrato, ping-pong delay, a per-mode room reverb, and an "auto voice" performer.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace monk
{

constexpr float kPi = 3.14159265358979323846f;

enum Mode { Monk = 0, Zombie = 1, Animatronic = 2 };

struct Formant { float freq, gainDb, bw; };
struct Vowel   { Formant f[5]; };

// Bass-singer formant table, ordered along the pad: U  O  A  E  I
inline const Vowel kVowels[5] = {
    {{{ 350.f,   0.f,  40.f }, { 600.f, -20.f,  80.f }, { 2400.f, -32.f, 100.f }, { 2675.f, -28.f, 120.f }, { 2950.f, -36.f, 120.f }}}, // U
    {{{ 400.f,   0.f,  40.f }, { 750.f, -11.f,  80.f }, { 2400.f, -21.f, 100.f }, { 2600.f, -20.f, 120.f }, { 2900.f, -40.f, 120.f }}}, // O
    {{{ 600.f,   0.f,  60.f }, {1040.f,  -7.f,  70.f }, { 2250.f,  -9.f, 110.f }, { 2450.f,  -9.f, 120.f }, { 2750.f, -20.f, 130.f }}}, // A
    {{{ 400.f,   0.f,  40.f }, {1620.f, -12.f,  80.f }, { 2400.f,  -9.f, 100.f }, { 2800.f, -12.f, 120.f }, { 3100.f, -18.f, 120.f }}}, // E
    {{{ 250.f,   0.f,  60.f }, {1750.f, -20.f,  90.f }, { 2600.f, -16.f, 100.f }, { 3050.f, -22.f, 120.f }, { 3340.f, -28.f, 120.f }}}, // I
};

// Per-vowel loudness trim (dB) so the sweep stays even
inline const float kVowelTrimDb[5] = { -4.5f, -1.0f, 0.f, -1.0f, -8.0f };

struct Params
{
    int   mode          = Monk;
    float vowel         = 0.5f;   // 0..1  (U..I)
    float padPitch      = 0.5f;   // 0..1  pitch when playing from the pad
    bool  padGate       = false;
    float glideMs       = 120.f;
    float vibRateHz     = 5.0f;
    float vibDepthCents = 30.f;
    float attackMs      = 80.f;
    float releaseMs     = 500.f;
    float formantShift  = 1.0f;   // "head size"
    float throat        = 0.1f;   // octave-down undertone
    float breath        = 0.12f;
    float growl         = 0.f;    // gravelly AM + drive
    float jitter        = 0.05f;  // unsteady pitch
    float robot         = 0.f;    // ring modulation
    float crush         = 0.f;    // amplitude quantising grit
    float delayMs       = 375.f;
    float delayFeedback = 0.4f;
    float delayMix      = 0.22f;
    float delayToneHz   = 3500.f;
    float room          = 0.4f;   // reverb send
    float gainDb        = 0.f;
};

//==============================================================================
class Resonator
{
public:
    void set (float freq, float bw, float sr)
    {
        freq = std::clamp (freq, 40.f, sr * 0.45f);
        const float w = 2.f * kPi * freq / sr;
        const float q = std::max (0.5f, freq / std::max (bw, 1.f));
        const float alpha = std::sin (w) / (2.f * q);
        const float a0 = 1.f + alpha;
        b0 = alpha / a0; b2 = -alpha / a0;
        a1 = -2.f * std::cos (w) / a0; a2 = (1.f - alpha) / a0;
    }
    float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = -a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() { z1 = z2 = 0.f; }
private:
    float b0 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
};

//==============================================================================
// Band-limited Rosenberg glottal-flow-derivative wavetables (one per octave of harmonics)
class GlottalTables
{
public:
    static constexpr int kSize = 2048, kLevels = 10;   // up to 512 harmonics

    static const GlottalTables& get() { static GlottalTables t; return t; }

    // phase 0..1, dt = f/sr
    float read (float phase, float dt) const
    {
        const int maxH = std::max (1, (int) (0.45f / std::max (dt, 1e-6f)));
        int level = 0;
        while (level + 1 < kLevels && (1 << (level + 1)) <= maxH) ++level;
        const float pos = phase * (float) kSize;
        const int i0 = (int) pos & (kSize - 1);
        const int i1 = (i0 + 1) & (kSize - 1);
        const float fr = pos - std::floor (pos);
        const auto& t = tables[(size_t) level];
        return t[(size_t) i0] + (t[(size_t) i1] - t[(size_t) i0]) * fr;
    }

private:
    GlottalTables()
    {
        constexpr int L = 4096, H = 1 << (kLevels - 1);
        std::vector<double> g (L), d (L);
        for (int i = 0; i < L; ++i)
        {
            const double t = (double) i / L;
            g[(size_t) i] = t < 0.42 ? 0.5 * (1.0 - std::cos (kPi * t / 0.42))
                          : t < 0.58 ? std::cos (kPi * 0.5 * (t - 0.42) / 0.16) : 0.0;
        }
        for (int i = 0; i < L; ++i) d[(size_t) i] = g[(size_t) i] - g[(size_t) ((i + L - 1) % L)];

        std::vector<double> re (H + 1), im (H + 1);
        for (int k = 1; k <= H; ++k)
        {
            double a = 0, b = 0;
            for (int i = 0; i < L; ++i)
            {
                const double w = 2.0 * kPi * k * i / L;
                a += d[(size_t) i] * std::cos (w);
                b += d[(size_t) i] * std::sin (w);
            }
            re[(size_t) k] = a; im[(size_t) k] = b;
        }

        double norm = 0;
        for (int lv = 0; lv < kLevels; ++lv)
        {
            const int nh = 1 << lv;
            auto& t = tables[(size_t) lv];
            t.assign (kSize, 0.f);
            for (int i = 0; i < kSize; ++i)
            {
                double s = 0;
                for (int k = 1; k <= nh; ++k)
                {
                    const double w = 2.0 * kPi * k * i / kSize;
                    s += re[(size_t) k] * std::cos (w) + im[(size_t) k] * std::sin (w);
                }
                t[(size_t) i] = (float) s;
                if (lv == kLevels - 1) norm = std::max (norm, std::abs (s));
            }
        }
        for (auto& t : tables) for (auto& v : t) v = (float) (v / norm);
    }

    std::vector<float> tables[kLevels];
};

//==============================================================================
// Small Freeverb-style stereo room
class Room
{
public:
    void prepare (float sampleRate)
    {
        static const int combT[8] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
        static const int apT[4]   = { 556, 441, 341, 225 };
        const float s = sampleRate / 44100.f;
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 8; ++i) { combs[ch][i].buf.assign ((size_t) ((combT[i] + ch * 23) * s), 0.f); combs[ch][i].pos = 0; combs[ch][i].store = 0; }
            for (int i = 0; i < 4; ++i) { aps[ch][i].buf.assign ((size_t) ((apT[i] + ch * 23) * s), 0.f); aps[ch][i].pos = 0; }
        }
        metal[0].assign ((size_t) (0.0031f * sampleRate), 0.f);
        metal[1].assign ((size_t) (0.0043f * sampleRate), 0.f);
        mPos[0] = mPos[1] = 0;
    }

    void setCharacter (int mode)
    {
        switch (mode)
        {
            case Zombie:      feedback = 0.80f; damp = 0.75f; metallic = 0.f;  break;  // dark crypt
            case Animatronic: feedback = 0.62f; damp = 0.15f; metallic = 0.6f; break;  // small tin room
            default:          feedback = 0.88f; damp = 0.25f; metallic = 0.f;  break;  // temple hall
        }
    }

    void process (float in, float& outL, float& outR)
    {
        in *= 0.015f;
        float o[2] = { 0.f, 0.f };
        for (int ch = 0; ch < 2; ++ch)
        {
            float x = in;
            if (metallic > 0.f)
            {
                auto& b = metal[ch]; int& p = mPos[ch];
                const float y = b[(size_t) p];
                b[(size_t) p] = x + y * metallic;
                if (++p >= (int) b.size()) p = 0;
                x = x + y;
            }
            float acc = 0.f;
            for (auto& c : combs[ch])
            {
                const float y = c.buf[(size_t) c.pos];
                c.store = y * (1.f - damp) + c.store * damp;
                c.buf[(size_t) c.pos] = x + c.store * feedback;
                if (++c.pos >= (int) c.buf.size()) c.pos = 0;
                acc += y;
            }
            for (auto& a : aps[ch])
            {
                const float b = a.buf[(size_t) a.pos];
                a.buf[(size_t) a.pos] = acc + b * 0.5f;
                if (++a.pos >= (int) a.buf.size()) a.pos = 0;
                acc = b - acc;
            }
            o[ch] = acc;
        }
        outL = o[0]; outR = o[1];
    }

private:
    struct Comb { std::vector<float> buf; int pos = 0; float store = 0; };
    struct AP   { std::vector<float> buf; int pos = 0; };
    Comb combs[2][8];
    AP aps[2][4];
    std::vector<float> metal[2];
    int mPos[2] { 0, 0 };
    float feedback = 0.88f, damp = 0.25f, metallic = 0.f;
};

//==============================================================================
inline float polyBlep (float t, float dt)
{
    if (t < dt)       { t /= dt;             return t + t - t * t - 1.f; }
    if (t > 1.f - dt) { t = (t - 1.f) / dt;  return t * t + t + t + 1.f; }
    return 0.f;
}

class MonkEngine
{
public:
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        (void) GlottalTables::get();
        const int maxDelay = (int) (sr * 2.1f) + 4;
        bufL.assign ((size_t) maxDelay, 0.f);
        bufR.assign ((size_t) maxDelay, 0.f);
        room.prepare (sr);
        reset();
    }

    void reset()
    {
        for (auto& r : res) r.reset();
        std::fill (bufL.begin(), bufL.end(), 0.f);
        std::fill (bufR.begin(), bufR.end(), 0.f);
        writePos = 0;
        env = 0.f; phase = subPhase = vibPhase = growlPhase = ringPhase = 0.f;
        tilt = fbLpL = fbLpR = hpX = hpY = 0.f;
        held.clear(); bend = 0.f; modWheel = 0.f;
        curVowel = p.vowel; curDelay = p.delayMs;
        coeffCounter = 0; autoActive = false;
    }

    void setParams (const Params& np) { p = np; room.setCharacter (p.mode); }

    void noteOn (int note, float)  { held.erase (std::remove (held.begin(), held.end(), note), held.end()); held.push_back (note); }
    void noteOff (int note)        { held.erase (std::remove (held.begin(), held.end(), note), held.end()); }
    void allNotesOff()             { held.clear(); }
    void setPitchBend (float semis){ bend = semis; }
    void setModWheel (float v01)   { modWheel = v01; }

    // Starts a short performed phrase in the style of the current mode
    void triggerAutoVoice()
    {
        autoActive = true; autoT = 0.f; autoMode = p.mode;
        const float r1 = nextRand01(), r2 = nextRand01(), r3 = nextRand01();
        switch (autoMode)
        {
            case Zombie:      autoDur = 1.3f + r1 * 0.8f; autoP0 = 0.34f + r2 * 0.1f; autoV0 = 0.05f + r3 * 0.08f; autoVPk = 0.22f + nextRand01() * 0.15f; break;
            case Animatronic: autoDur = 2.2f; break;
            default:          autoDur = 3.5f; break;
        }
    }

    void render (float* L, float* R, int n)
    {
        const float glideCoef = coefFor (p.glideMs);
        const float atkCoef   = coefFor (p.attackMs);
        const float relCoef   = coefFor (p.releaseMs);
        const float vowCoef   = coefFor (30.f);
        const float dlyCoef   = coefFor (80.f);
        const float rndCoef   = coefFor (25.f);
        const float outGain   = std::pow (10.f, p.gainDb / 20.f);
        const float fbCoef    = 1.f - std::exp (-2.f * kPi * p.delayToneHz / sr);
        const float tiltCoef  = 1.f - std::exp (-2.f * kPi * 2600.f / sr);
        const float hpCoef    = std::exp (-2.f * kPi * 120.f / sr);
        const int   bufSize   = (int) bufL.size();
        const auto& glot      = GlottalTables::get();

        for (int i = 0; i < n; ++i)
        {
            // ---- auto voice performer (overrides pad)
            float padVowel = p.vowel, padPitch = p.padPitch;
            bool padGate = p.padGate;
            if (autoActive)
            {
                autoT += 1.f / sr;
                const float u = autoT / autoDur;
                if (u >= 1.f) autoActive = false;
                else { autoPerform (u, padVowel, padPitch); padGate = true; }
            }
            dispPitch = padPitch;

            // ---- gate / target pitch
            bool gate = false;
            float targetNote = curNote;
            if (! held.empty()) { gate = true; targetNote = (float) held.back(); dispPitch = std::clamp ((targetNote - 33.f) / 24.f, 0.f, 1.f); }
            else if (padGate)   { gate = true; targetNote = 33.f + padPitch * 24.f; }

            if (gate && ! wasGate)
            {
                vibOnset = 0.f;
                if (env < 0.01f || p.glideMs < 1.f) curNote = targetNote;
            }
            wasGate = gate;

            curNote += (targetNote - curNote) * glideCoef;
            env     += ((gate ? 1.f : 0.f) - env) * (gate ? atkCoef : relCoef);

            // ---- natural pitch drift / shimmer (random walks, retargeted every ~50 ms)
            if (--rndCounter <= 0)
            {
                rndCounter = (int) (sr * (0.03f + 0.05f * nextRand01()));
                jitTarget = nextRand01() * 2.f - 1.f;
                shimTarget = nextRand01() * 2.f - 1.f;
            }
            jitVal  += (jitTarget  - jitVal)  * rndCoef;
            shimVal += (shimTarget - shimVal) * rndCoef;

            // ---- vibrato (fades in after onset)
            vibOnset = std::min (1.f, vibOnset + 1.f / (0.4f * sr));
            vibPhase += p.vibRateHz / sr;
            if (vibPhase >= 1.f) vibPhase -= 1.f;
            const float vib = std::sin (2.f * kPi * vibPhase) * (p.vibDepthCents / 100.f) * vibOnset;
            const float drift = jitVal * (6.f + 45.f * p.jitter) / 100.f;

            const float note = curNote + bend + vib + drift;
            const float freq = 440.f * std::pow (2.f, (note - 69.f) / 12.f);

            // ---- glottal source + throat undertone
            const float dt = std::min (freq / sr, 0.5f);
            phase += dt; if (phase >= 1.f) phase -= 1.f;
            const float sdt = dt * 0.5f;
            subPhase += sdt; if (subPhase >= 1.f) subPhase -= 1.f;
            float src = glot.read (phase, dt) * (1.f - 0.5f * p.throat) + glot.read (subPhase, sdt) * p.throat;

            tilt += (src - tilt) * tiltCoef;
            src = tilt * 2.2f * (1.f + 0.04f * shimVal) + nextNoise() * p.breath * 0.3f;

            // ---- formants
            curVowel += (std::clamp (padVowel + modWheel, 0.f, 1.f) - curVowel) * vowCoef;
            if (coeffCounter-- <= 0) { updateFormants(); coeffCounter = 16; }

            float x = 0.f;
            for (int k = 0; k < 5; ++k) x += res[k].process (src) * fGain[k];
            x *= env;

            // ---- growl: rough amplitude flutter + drive
            if (p.growl > 0.f)
            {
                growlPhase += (28.f + freq * 0.12f) / sr;
                if (growlPhase >= 1.f) growlPhase -= 1.f;
                const float tri = 1.f - 4.f * std::abs (growlPhase - 0.5f);   // -1..1
                x *= 1.f - 0.45f * p.growl + 0.45f * p.growl * tri;
                x = x * (1.f - 0.5f * p.growl) + std::tanh (x * 3.f) * 0.6f * p.growl;
            }

            // ---- robot: ring modulation at 1.5x pitch
            if (p.robot > 0.f)
            {
                ringPhase += freq * 1.5f / sr;
                if (ringPhase >= 1.f) ringPhase -= 1.f;
                const float sq = std::tanh (8.f * std::sin (2.f * kPi * ringPhase));
                x = x * (1.f - 0.8f * p.robot) + x * sq * 1.3f * p.robot;
            }

            // ---- crush: amplitude quantising + high-pass
            if (p.crush > 0.f)
            {
                const float q = std::round (std::clamp (x * (1.f + 2.f * p.crush), -1.f, 1.f) * 5.f) / 5.f;
                hpY = hpCoef * (hpY + q - hpX); hpX = q;
                x = x * (1.f - 0.7f * p.crush) + hpY * 0.5f * p.crush;
            }

            // ---- ping-pong delay
            curDelay += (p.delayMs - curDelay) * dlyCoef;
            const float dSamps = std::clamp (curDelay * 0.001f * sr, 1.f, (float) bufSize - 2.f);
            const float dl = readFrac (bufL, dSamps);
            const float dr = readFrac (bufR, dSamps);
            fbLpL += (dl - fbLpL) * fbCoef;
            fbLpR += (dr - fbLpR) * fbCoef;
            bufL[(size_t) writePos] = x + fbLpR * p.delayFeedback;
            bufR[(size_t) writePos] = fbLpL * p.delayFeedback;
            if (++writePos >= bufSize) writePos = 0;

            const float dry = 1.f - 0.5f * p.delayMix;
            float l = x * dry + dl * p.delayMix;
            float r = x * dry + dr * p.delayMix;

            // ---- room
            float rl, rr;
            room.process (l + r, rl, rr);
            l += rl * p.room;
            r += rr * p.room;

            L[i] = std::tanh (l * outGain);
            R[i] = std::tanh (r * outGain);
        }
    }

    float getCurrentVowel() const { return curVowel; }
    float getEnvelope()     const { return env; }
    float getDisplayPitch() const { return dispPitch; }
    bool  isAutoActive()    const { return autoActive; }

private:
    void autoPerform (float u, float& v, float& pitch)
    {
        if (autoMode == Zombie)
        {
            pitch = u < 0.18f ? autoP0 + 0.06f * (u / 0.18f) : autoP0 + 0.06f - 0.2f * std::pow ((u - 0.18f) / 0.82f, 1.4f);
            v = u < 0.35f ? autoV0 + (autoVPk - autoV0) * (u / 0.35f) : autoVPk - (autoVPk - autoV0 * 0.6f) * ((u - 0.35f) / 0.65f);
        }
        else if (autoMode == Animatronic)
        {
            static const float seq[6] = { .45f, .45f, .5f, .38f, .55f, .3f };
            static const float vs[6]  = { .5f, .15f, .75f, .5f, .25f, .6f };
            const int k = std::min (5, (int) (u * 6.f));
            pitch = seq[k] - u * 0.12f; v = vs[k];
        }
        else
        {
            pitch = 0.42f + 0.06f * std::sin (u * kPi);
            v = u < 0.7f ? 0.05f + 0.4f * (u / 0.7f) : 0.45f - 0.4f * ((u - 0.7f) / 0.3f);
        }
        pitch = std::clamp (pitch, 0.f, 1.f);
    }

    float coefFor (float ms) const
    {
        return ms < 0.5f ? 1.f : 1.f - std::exp (-1.f / (ms * 0.001f * sr / 4.6f));
    }

    void updateFormants()
    {
        const float pos = curVowel * 4.f;
        const int a = std::min (3, (int) pos);
        const float t = pos - (float) a;
        const Vowel& va = kVowels[a];
        const Vowel& vb = kVowels[a + 1];
        const float bwScale = p.formantShift * (1.f + 0.6f * p.growl);
        for (int k = 0; k < 5; ++k)
        {
            const float f  = std::exp (std::log (va.f[k].freq) * (1.f - t) + std::log (vb.f[k].freq) * t) * p.formantShift;
            const float bw = (va.f[k].bw * (1.f - t) + vb.f[k].bw * t) * bwScale;
            const float db = va.f[k].gainDb * (1.f - t) + vb.f[k].gainDb * t
                           + kVowelTrimDb[a] * (1.f - t) + kVowelTrimDb[a + 1] * t;
            res[k].set (f, bw, sr);
            fGain[k] = std::pow (10.f, db / 20.f);
        }
    }

    float readFrac (const std::vector<float>& b, float delay) const
    {
        float rp = (float) writePos - delay;
        const int size = (int) b.size();
        if (rp < 0.f) rp += (float) size;
        const int i0 = (int) rp;
        const int i1 = (i0 + 1) % size;
        const float fr = rp - (float) i0;
        return b[(size_t) i0] + (b[(size_t) i1] - b[(size_t) i0]) * fr;
    }

    float nextNoise()     { rng = rng * 1664525u + 1013904223u; return (float) (rng >> 8) / 8388608.f - 1.f; }
    float nextRand01()    { return 0.5f * (nextNoise() + 1.f); }

    Params p;
    float sr = 44100.f;
    Resonator res[5];
    float fGain[5] {};
    Room room;
    std::vector<float> bufL, bufR;
    int writePos = 0;
    std::vector<int> held;
    float curNote = 45.f, bend = 0.f, modWheel = 0.f;
    float env = 0.f, phase = 0.f, subPhase = 0.f, vibPhase = 0.f, vibOnset = 0.f;
    float growlPhase = 0.f, ringPhase = 0.f, hpX = 0.f, hpY = 0.f;
    float tilt = 0.f, fbLpL = 0.f, fbLpR = 0.f;
    float jitVal = 0.f, jitTarget = 0.f, shimVal = 0.f, shimTarget = 0.f;
    int rndCounter = 0;
    float curVowel = 0.5f, curDelay = 375.f, dispPitch = 0.5f;
    int coeffCounter = 0;
    bool wasGate = false;
    bool autoActive = false; int autoMode = 0;
    float autoT = 0.f, autoDur = 1.f, autoP0 = 0.4f, autoV0 = 0.1f, autoVPk = 0.3f;
    uint32_t rng = 22222u;
};

} // namespace monk
