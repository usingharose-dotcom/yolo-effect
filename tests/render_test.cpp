// Offline render test (no JUCE): g++ -O2 -std=c++17 -ISource tests/render_test.cpp -o render_test
// Renders demo.wav: each mode plays a held note with a vowel sweep, then its auto voice.
#include "MonkEngine.h"
#include <cstdio>
#include <cmath>
#include <vector>

static void writeWav (const char* path, const std::vector<float>& L, const std::vector<float>& R, int sr)
{
    FILE* f = std::fopen (path, "wb");
    if (! f) return;
    const int n = (int) L.size(), bytes = n * 4;
    auto u32 = [&] (uint32_t v) { std::fwrite (&v, 4, 1, f); };
    auto u16 = [&] (uint16_t v) { std::fwrite (&v, 2, 1, f); };
    std::fwrite ("RIFF", 1, 4, f); u32 (36 + bytes); std::fwrite ("WAVEfmt ", 1, 8, f);
    u32 (16); u16 (1); u16 (2); u32 (sr); u32 (sr * 4); u16 (4); u16 (16);
    std::fwrite ("data", 1, 4, f); u32 (bytes);
    for (int i = 0; i < n; ++i)
    {
        int16_t l = (int16_t) std::lrint (std::fmax (-1.f, std::fmin (1.f, L[i])) * 32767.f);
        int16_t r = (int16_t) std::lrint (std::fmax (-1.f, std::fmin (1.f, R[i])) * 32767.f);
        std::fwrite (&l, 2, 1, f); std::fwrite (&r, 2, 1, f);
    }
    std::fclose (f);
}

static monk::Params preset (int mode)
{
    monk::Params p; p.mode = mode;
    if (mode == monk::Zombie)
    { p.glideMs=320; p.vibRateHz=3; p.vibDepthCents=0; p.formantShift=.82f; p.throat=.55f; p.attackMs=140; p.releaseMs=350; p.breath=.45f; p.growl=.75f; p.jitter=.6f; p.delayMs=90; p.delayFeedback=.15f; p.delayToneHz=1500; p.delayMix=.1f; p.room=.3f; }
    if (mode == monk::Animatronic)
    { p.glideMs=40; p.vibRateHz=7; p.vibDepthCents=8; p.formantShift=.9f; p.throat=.3f; p.attackMs=20; p.releaseMs=250; p.breath=.05f; p.growl=.15f; p.jitter=0; p.robot=.65f; p.crush=.45f; p.delayMs=240; p.delayFeedback=.35f; p.delayToneHz=2500; p.delayMix=.25f; p.room=.3f; }
    return p;
}

int main()
{
    const int sr = 48000, block = 256;
    std::vector<float> L, R;
    float bl[block], br[block];
    bool bad = false;

    for (int mode = 0; mode < 3; ++mode)
    {
        monk::MonkEngine e;
        e.prepare (sr);
        monk::Params p = preset (mode);
        e.setParams (p);
        const int nBlocks = (int) (8.f * sr / block);
        float peak = 0.f, sumSq = 0.f;
        for (int b = 0; b < nBlocks; ++b)
        {
            const float t = (float) b * block / sr;
            if (b == 0) e.noteOn (45, 1.f);
            if (t >= 3.f && t < 3.f + (float) block / sr) e.noteOff (45);
            if (t >= 4.5f && t < 4.5f + (float) block / sr) e.triggerAutoVoice();
            p.vowel = 0.5f - 0.5f * std::cos (t * 1.6f);
            e.setParams (p);
            e.render (bl, br, block);
            for (int i = 0; i < block; ++i)
            {
                if (! std::isfinite (bl[i]) || ! std::isfinite (br[i])) bad = true;
                peak = std::fmax (peak, std::fabs (bl[i])); sumSq += bl[i] * bl[i];
            }
            L.insert (L.end(), bl, bl + block);
            R.insert (R.end(), br, br + block);
        }
        std::printf ("mode %d: peak %.3f  rms %.1f dB\n", mode, peak, 10.f * std::log10 (sumSq / (nBlocks * block) + 1e-12f));
    }
    writeWav ("demo.wav", L, R, sr);
    std::printf (bad ? "NON-FINITE SAMPLES!\n" : "all samples finite\n");
    return bad ? 1 : 0;
}
