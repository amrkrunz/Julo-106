#include "../Source/Engine.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace juno;

static void defaults(float* p) { for (int i = 0; i < P::COUNT; ++i) p[i] = paramDefs()[i].def; }

static int render(const char* path, float* p, std::vector<int> notes, float seconds, double sr = 48000.0, bool release = false)
{
    Engine e; e.prepare(sr); e.setParams(p);
    for (int n : notes) e.noteOn(n, 1.f);
    int total = (int)(seconds * sr); std::vector<float> L(total), R(total);
    int half = total / 2;
    e.process(L.data(), R.data(), half);
    if (release) for (int n : notes) e.noteOff(n);
    e.process(L.data() + half, R.data() + half, total - half);
    int bad = 0; float peak = 0.f;
    for (int i = 0; i < total; ++i) { if (!std::isfinite(L[i]) || !std::isfinite(R[i])) ++bad; peak = std::max(peak, std::max(std::fabs(L[i]), std::fabs(R[i]))); }
    FILE* f = fopen(path, "wb"); for (int i = 0; i < total; ++i) { float s[2] = { L[i], R[i] }; fwrite(s, sizeof(float), 2, f); } fclose(f);
    printf("%-28s peak=%.3f nonfinite=%d\n", path, peak, bad);
    return bad;
}

int main()
{
    int fails = 0; float p[P::COUNT];

    // 1. default patch, 6-note chord
    defaults(p); fails += render("/tmp/default_chord.f32", p, { 48, 52, 55, 60, 64, 67 }, 2.0f);
    // 2. pure saw, filter open, high note: aliasing check
    defaults(p); p[P::pulse] = 0; p[P::cutoff] = 1; p[P::res] = 0; p[P::envAmt] = 0; p[P::keyFollow] = 0; p[P::sustain] = 1; p[P::attack] = 0;
    fails += render("/tmp/saw_c7.f32", p, { 96 }, 1.0f);
    // 3. max resonance, self-oscillation stability
    defaults(p); p[P::res] = 1.f; p[P::cutoff] = 0.5f; p[P::saw] = 0; p[P::pulse] = 0; p[P::noise] = 0.05f; p[P::sustain] = 1;
    fails += render("/tmp/selfosc.f32", p, { 60 }, 2.0f);
    // 4. everything on: chorus III, sat, comp, reverb, unison, porta, gate, lfo
    defaults(p); p[P::chorus] = 3; p[P::satDrive] = 1; p[P::compOn] = 1; p[P::revMix] = 0.5f; p[P::revSize] = 1; p[P::voiceMode] = 1;
    p[P::porta] = 0.4f; p[P::dcoLfo] = 0.5f; p[P::lfoVcf] = 0.5f; p[P::pwmMode] = 1; p[P::dcoPwm] = 0.8f; p[P::vcaMode] = 1; p[P::vcfPol] = 1; p[P::lfoDelay] = 0.5f;
    fails += render("/tmp/everything.f32", p, { 40, 47, 52 }, 3.0f, 44100.0, true);
    // 5. reverb tail decay, size 0 and 1
    for (float sz : { 0.f, 1.f }) { defaults(p); p[P::revMix] = 1.f; p[P::revSize] = sz; p[P::attack] = 0; p[P::release] = 0; p[P::decay] = 0; p[P::sustain] = 0;
        fails += render(sz < 0.5f ? "/tmp/rev_small.f32" : "/tmp/rev_big.f32", p, { 60 }, 4.0f); }
    // 6. envelope timing: attack slider 0.5, sustain 1, VCF fully open, report 90% rise time
    { defaults(p); p[P::attack] = 0.5f; p[P::sustain] = 1; p[P::cutoff] = 1; p[P::envAmt] = 0; p[P::saw] = 0; p[P::pulse] = 0; p[P::sub] = 1;
      Engine e; e.prepare(48000); e.setParams(p); e.noteOn(60, 1); std::vector<float> L(96000), R(96000); e.process(L.data(), R.data(), 96000);
      float mx = 0; for (float v : L) mx = std::max(mx, std::fabs(v)); int t90 = -1; for (int i = 0; i < 96000; ++i) if (std::fabs(L[i]) > 0.9f * mx) { t90 = i; break; }
      printf("attack(0.5) ~ %.3f s to 90%% (expect ~0.07-0.2s)\n", t90 / 48000.0); }
    // 7. standalone ladder frequency response
    { const double sr = 48000; const float fc = 1000.f; const float g = std::tan(kPi * fc / (float)sr);
      for (float k : { 0.f, 3.5f }) { printf("ladder k=%.1f:", k);
        for (float f : { 250.f, 500.f, 1000.f, 2000.f, 4000.f, 8000.f }) {
            Ladder l; double acc = 0; int N = 48000;
            for (int i = 0; i < N; ++i) { float x = 0.05f * std::sin(kTwoPi * f * (float)i / (float)sr); float y = l.tick(x, g, k); if (i > N / 2) acc += y * y; }
            printf(" %5.0fHz:%6.1fdB", f, 10 * std::log10(acc / (N / 2) / (0.05 * 0.05 / 2))); }
        printf("\n"); } }
    printf(fails ? "FAILURES: %d\n" : "ALL RENDERS FINITE\n", fails);
    return fails != 0;
}
