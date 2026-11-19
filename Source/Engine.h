#pragma once
// J-106 DSP engine: JUCE-free, header-only. Juno-106 style voice architecture
// (single DCO + sub + noise -> HPF -> 24 dB/oct OTA-ladder LPF -> VCA, one ADSR,
// triangle LFO, BBD-style stereo chorus) followed by saturation, compressor, reverb.
#include "Params.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace juno {

constexpr float kPi    = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;

inline float fastTanh(float x)
{
    x = std::max(-3.f, std::min(3.f, x));
    const float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}
inline float dbToLin(float d) { return std::pow(10.f, d * 0.05f); }

struct Rng {
    uint32_t s = 0x1234567u;
    float next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (float)(int32_t)s * (1.0f / 2147483648.0f); }
};

inline float polyBlep(float t, float dt)
{
    if (t < dt)        { t /= dt;               return t + t - t * t - 1.f; }
    if (t > 1.f - dt)  { t = (t - 1.f) / dt;    return t * t + t + t + 1.f; }
    return 0.f;
}

struct DelayLine {
    std::vector<float> b; int mask = 0, w = 0;
    void init(int maxSamples) { int n = 1; while (n < maxSamples + 8) n <<= 1; b.assign((size_t)n, 0.f); mask = n - 1; w = 0; }
    void push(float x) { b[(size_t)w] = x; w = (w + 1) & mask; }
    float read(float d) const
    {
        const int di = (int)d; const float fr = d - (float)di;
        const int i0 = (w - di) & mask;
        return b[(size_t)i0] * (1.f - fr) + b[(size_t)((i0 - 1) & mask)] * fr;
    }
};

// ---------------------------------------------------------------- envelope
// Analog-style RC envelope: attack charges toward an over-target, decay/release are exponential.
struct Adsr {
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle; float level = 0.f, aC = 0.f, dC = 0.f, rC = 0.f, sus = 0.5f;
    void set(float a01, float d01, float s, float r01, float sr)
    {
        auto tm = [](float x, float lo, float hi) { return lo * std::pow(hi / lo, x); };
        const float ta = tm(a01, 0.0015f, 3.f), td = tm(d01, 0.0015f, 12.f), tr = tm(r01, 0.0015f, 12.f);
        aC = std::exp(-1.79f / (ta * sr)); dC = std::exp(-4.6f / (td * sr)); rC = std::exp(-4.6f / (tr * sr)); sus = s;
    }
    void gateOn()  { stage = Attack; }
    void gateOff() { if (stage != Idle) stage = Release; }
    void reset()   { stage = Idle; level = 0.f; }
    float tick()
    {
        switch (stage) {
            case Attack:  level = 1.2f + (level - 1.2f) * aC; if (level >= 1.f) { level = 1.f; stage = Decay; } break;
            case Decay:   level = sus + (level - sus) * dC;   if (level <= sus + 1e-4f) { level = sus; stage = Sustain; } break;
            case Sustain: level += (sus - level) * 0.002f; break;
            case Release: level *= rC; if (level < 1e-4f) { level = 0.f; stage = Idle; } break;
            default: level = 0.f;
        }
        return level;
    }
};

// ---------------------------------------------------------------- 4-pole OTA ladder (zero-delay feedback)
struct Ladder {
    float s[4] = { 0, 0, 0, 0 };
    void reset() { s[0] = s[1] = s[2] = s[3] = 0.f; }
    float tick(float x, float g, float k)
    {
        const float G = g / (1.f + g), G2 = G * G, G4 = G2 * G2, iG = 1.f - G;
        const float Sg = G * G2 * iG * s[0] + G2 * iG * s[1] + G * iG * s[2] + iG * s[3];
        const float y4 = (G4 * x + Sg) / (1.f + k * G4);
        float in = fastTanh(x - k * y4);
        for (int i = 0; i < 4; ++i) {
            const float v = (in - s[i]) * G, y = v + s[i];
            s[i] = y + v;
            in = (i < 3) ? fastTanh(y) : y;
        }
        return in;
    }
};

// ---------------------------------------------------------------- BBD-style stereo chorus
struct Chorus {
    DelayLine dl; float sr = 48000.f, phase = 0.f;
    float rate = 0.5f, depth = 0.001f, wet = 0.f, rateT = 0.5f, depthT = 0.001f, wetT = 0.f;
    float lpA = 0.5f, l1 = 0, l2 = 0, r1 = 0, r2 = 0;
    void prepare(float s) { sr = s; dl.init((int)(0.03f * sr)); lpA = 1.f - std::exp(-kTwoPi * 7500.f / sr); phase = 0; l1 = l2 = r1 = r2 = 0; }
    void setMode(int m)
    {
        switch (m) {
            case 0: wetT = 0.f; break;
            case 1: wetT = 1.f; rateT = 0.513f; depthT = 0.00085f; break;   // Chorus I
            case 2: wetT = 1.f; rateT = 0.83f;  depthT = 0.00105f; break;   // Chorus II
            default: wetT = 1.f; rateT = 9.0f;  depthT = 0.00030f; break;   // I + II
        }
    }
    void process(float in, float& oL, float& oR)
    {
        const float sm = 1.f - std::exp(-1.f / (0.05f * sr));
        rate += (rateT - rate) * sm; depth += (depthT - depth) * sm; wet += (wetT - wet) * sm;
        phase += rate / sr; if (phase >= 1.f) phase -= 1.f;
        const float tri = 4.f * std::fabs(phase - 0.5f) - 1.f, base = 0.0034f;
        dl.push(in);
        float wl = dl.read((base + depth * tri) * sr), wr = dl.read((base - depth * tri) * sr);
        l1 += lpA * (wl - l1); l2 += lpA * (l1 - l2); wl = l2;     // BBD / anti-alias band limiting
        r1 += lpA * (wr - r1); r2 += lpA * (r1 - r2); wr = r2;
        wl = fastTanh(wl * 1.5f) / 1.5f; wr = fastTanh(wr * 1.5f) / 1.5f;
        const float dryG = 1.f - 0.3f * wet, wetG = 0.7f * wet;
        oL = in * dryG + wl * wetG; oR = in * dryG + wr * wetG;
    }
};

// ---------------------------------------------------------------- saturation (tanh with 1st-order ADAA)
struct Saturator {
    float x1 = 0.f, F1 = 0.f;
    static float logcosh(float x) { const float a = std::fabs(x); return a + std::log1p(std::exp(-2.f * a)) - 0.69314718f; }
    float tick(float x, float pre, float comp)
    {
        const float xi = x * pre, F0 = logcosh(xi), dx = xi - x1;
        const float y = (std::fabs(dx) < 1e-4f) ? std::tanh(0.5f * (xi + x1)) : (F0 - F1) / dx;
        x1 = xi; F1 = F0;
        return y * comp;
    }
};

// ---------------------------------------------------------------- compressor (stereo-linked, soft knee)
struct Compressor {
    float sr = 48000.f, g = 0.f, aA = 0.f, aR = 0.f, thr = -18.f, ratio = 4.f, makeup = 0.f;
    void prepare(float s) { sr = s; g = 0.f; }
    void set(float t, float r, float atkMs, float relMs, float mk)
    {
        thr = t; ratio = std::max(1.f, r); makeup = mk;
        aA = std::exp(-1.f / (0.001f * atkMs * sr)); aR = std::exp(-1.f / (0.001f * relMs * sr));
    }
    void process(float& l, float& r)
    {
        const float peak = std::max(std::fabs(l), std::fabs(r));
        const float over = 20.f * std::log10(peak + 1e-9f) - thr, knee = 6.f, slope = 1.f - 1.f / ratio;
        float gr;
        if (over <= -knee * 0.5f) gr = 0.f;
        else if (over >= knee * 0.5f) gr = over * slope;
        else { const float t = over + knee * 0.5f; gr = slope * t * t / (2.f * knee); }
        g = (gr > g) ? aA * g + (1.f - aA) * gr : aR * g + (1.f - aR) * gr;
        const float gain = dbToLin(makeup - g);
        l *= gain; r *= gain;
    }
};

// ---------------------------------------------------------------- reverb (8-line FDN, Householder mix)
struct Reverb {
    static constexpr int N = 8;
    DelayLine dl[N], ap[4];
    float lp[N] = {}, len[N] = {}, fbg[N] = {}, ph[N] = {}, phInc[N] = {}, apLen[4] = {};
    float dampA = 0.5f, sr = 48000.f, modSamples = 6.f, outGain = 1.f;
    static constexpr float baseMs[N] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.3f, 71.9f };
    void prepare(float s)
    {
        sr = s; modSamples = 0.00012f * sr;
        const float apMs[4] = { 5.1f, 7.7f, 11.3f, 15.1f };
        for (int i = 0; i < N; ++i) { dl[i].init((int)(0.2f * sr)); lp[i] = 0.f; ph[i] = 0.37f * (float)i - std::floor(0.37f * (float)i); phInc[i] = (0.07f + 0.031f * (float)i) / sr; }
        for (int i = 0; i < 4; ++i) { ap[i].init((int)(0.03f * sr)); apLen[i] = apMs[i] * 0.001f * sr; }
        setParams(0.5f, 0.5f);
    }
    void setParams(float size, float damp)
    {
        const float scale = 0.7f + 0.8f * size, rt60 = 0.3f * std::pow(30.f, size);
        for (int i = 0; i < N; ++i) { len[i] = baseMs[i] * 0.001f * sr * scale; fbg[i] = std::pow(10.f, -3.f * len[i] / (rt60 * sr)); }
        outGain = 5.f * std::pow(rt60, -0.375f);   // keeps wet level roughly constant across sizes
        dampA = 1.f - std::exp(-kTwoPi * 1200.f * std::exp2((1.f - damp) * 4.f) / sr);
    }
    void process(float inL, float inR, float& oL, float& oR)
    {
        float x = 0.5f * (inL + inR);
        for (int k = 0; k < 4; ++k) { const float d = ap[k].read(apLen[k]), v = x + 0.6f * d; ap[k].push(v); x = d - 0.6f * v; }
        float y[N], sum = 0.f;
        for (int i = 0; i < N; ++i) {
            const float m = std::sin(kTwoPi * ph[i]); ph[i] += phInc[i]; if (ph[i] >= 1.f) ph[i] -= 1.f;
            const float s = dl[i].read(len[i] + modSamples * m);
            lp[i] += dampA * (s - lp[i]); y[i] = lp[i] * fbg[i]; sum += y[i];
        }
        const float h = sum * (2.f / (float)N);
        for (int i = 0; i < N; ++i) dl[i].push(y[i] - h + x * 0.35f);
        oL = (y[0] - y[1] + y[2] - y[3] + y[4] - y[5] + y[6] - y[7]) * 0.4f * outGain;
        oR = (y[0] + y[1] - y[2] - y[3] + y[4] + y[5] - y[6] - y[7]) * 0.4f * outGain;
    }
};

// ---------------------------------------------------------------- voice + engine
struct Voice {
    bool active = false, gate = false, sustained = false;
    int note = 60; float velGain = 1.f; uint32_t age = 0;
    float pitch = 60.f, target = 60.f, detune = 0.f;
    float ph = 0.f; int subFlip = 0;
    Adsr env; Ladder lad; float hpLp = 0.f, vca = 0.f, cutVar = 1.f;
};

class Engine {
public:
    static constexpr int kVoices = 6;

    void prepare(double sampleRate)
    {
        sr = (float)sampleRate;
        Rng r; r.s = 0xBEEF123u;
        for (int i = 0; i < kVoices; ++i) { voices[i] = Voice(); voices[i].cutVar = 1.f + 0.012f * r.next(); }
        chorus.prepare(sr); comp.prepare(sr); rev.prepare(sr);
        satL = Saturator(); satR = Saturator();
        lfoPhase = 0.f; lfoTime = 1e9f; bendSm = bend = 0.f; modSm = mod = 0.f;
        gateCoef = 1.f - std::exp(-1.f / (0.0015f * sr));
        for (int i = 0; i < P::COUNT; ++i) p[i] = paramDefs()[i].def;
        setParams(p);
        cutSm = p[P::cutoff]; resSm = p[P::res]; lvlSm = p[P::vcaLevel]; masterSm = p[P::master];
    }

    void setParams(const float* v)
    {
        for (int i = 0; i < P::COUNT; ++i) p[i] = v[i];
        static const float rm[3] = { 0.5f, 1.f, 2.f };
        rangeMul = rm[(int)p[P::range]];
        hpfMode = (int)(p[P::hpf] + 0.5f);
        const float hpfHz = hpfMode == 0 ? 100.f : (hpfMode == 2 ? 240.f : 720.f);
        aHpf = 1.f - std::exp(-kTwoPi * hpfHz / sr);
        for (auto& vo : voices) vo.env.set(p[P::attack], p[P::decay], p[P::sustain], p[P::release], sr);
        lfoHz = 0.3f * std::pow(83.3f, p[P::lfoRate]);
        lfoDelaySec = 3.f * p[P::lfoDelay] * p[P::lfoDelay];
        const float pt = p[P::porta];
        glideCoef = pt < 0.002f ? 1.f : 1.f - std::exp(-1.f / ((0.005f * std::pow(400.f, pt)) * sr / 3.f));
        chorus.setMode((int)p[P::chorus]);
        comp.set(p[P::compThresh], p[P::compRatio], p[P::compAttack], p[P::compRelease], p[P::compMakeup]);
        rev.setParams(p[P::revSize], p[P::revDamp]);
        const int vm = (int)p[P::voiceMode];
        if (vm != mode) { allNotesOff(); mode = vm; }
    }

    void noteOn(int note, float vel01)
    {
        bool anyHeld = false;
        for (auto& v : voices) anyHeld = anyHeld || (v.active && v.gate);
        if (!anyHeld) lfoTime = 0.f;
        const float vg = 1.f - p[P::velocity] * (1.f - vel01);
        if (mode == 1) {
            stack.erase(std::remove(stack.begin(), stack.end(), note), stack.end());
            stack.push_back(note);
            for (int i = 0; i < kVoices; ++i) trigger(voices[i], note, vg, ((float)i - 2.5f) * 1.0f);
        } else {
            Voice* t = nullptr;
            for (auto& v : voices) if (v.active && v.note == note) { t = &v; break; }
            if (!t) for (auto& v : voices) if (!v.active) { t = &v; break; }
            if (!t) {
                uint32_t best = 0xFFFFFFFFu;
                for (auto& v : voices) if (!v.gate && v.age < best) { best = v.age; t = &v; }
                if (!t) { best = 0xFFFFFFFFu; for (auto& v : voices) if (v.age < best) { best = v.age; t = &v; } }
            }
            trigger(*t, note, vg, 0.f);
        }
        lastNote = (float)note;
    }

    void noteOff(int note)
    {
        if (mode == 1) {
            stack.erase(std::remove(stack.begin(), stack.end(), note), stack.end());
            if (!stack.empty()) { for (auto& v : voices) { v.target = (float)stack.back(); v.note = stack.back(); if (glideCoef >= 1.f) v.pitch = v.target; } }
            else for (auto& v : voices) release(v);
        } else {
            for (auto& v : voices) if (v.active && v.gate && v.note == note) { if (pedal) v.sustained = true; else release(v); }
        }
    }

    void allNotesOff() { stack.clear(); pedal = false; for (auto& v : voices) { v.sustained = false; release(v); } }
    void pitchBend(float b) { bend = std::max(-1.f, std::min(1.f, b)); }
    void modWheel(float m)  { mod = std::max(0.f, std::min(1.f, m)); }
    void sustainPedal(bool on)
    {
        pedal = on;
        if (!on) for (auto& v : voices) if (v.sustained) { v.sustained = false; release(v); }
    }

    void process(float* outL, float* outR, int n)
    {
        const float sm = 1.f - std::exp(-1.f / (0.005f * sr));
        const float tuneSemis = p[P::tune] * 0.01f, bendRange = p[P::bendRange];
        const bool pwmLfo = p[P::pwmMode] > 0.5f, gateMode = p[P::vcaMode] > 0.5f;
        const float pol = p[P::vcfPol] > 0.5f ? -1.f : 1.f;
        const float sawOn = p[P::saw] > 0.5f ? 1.f : 0.f, pulseOn = p[P::pulse] > 0.5f ? 1.f : 0.f;
        const float pwm = p[P::dcoPwm], subLvl = p[P::sub], noiseLvl = p[P::noise], envAmt = p[P::envAmt];
        const float keyFollow = p[P::keyFollow], dcoLfo = p[P::dcoLfo], lfoVcf = p[P::lfoVcf];
        const float modPitch = p[P::modPitch], modFilter = p[P::modFilter];
        const float satPre = 1.f + p[P::satDrive] * 9.f, satComp = 1.f / std::sqrt(satPre), satMix = p[P::satMix];
        const bool satOn = p[P::satDrive] > 0.001f, compOn = p[P::compOn] > 0.5f;
        const float revMix = p[P::revMix], revDry = std::cos(revMix * kPi * 0.5f), revWet = std::sin(revMix * kPi * 0.5f);

        for (int i = 0; i < n; ++i) {
            cutSm += (p[P::cutoff] - cutSm) * sm; resSm += (p[P::res] - resSm) * sm;
            lvlSm += (p[P::vcaLevel] - lvlSm) * sm; masterSm += (p[P::master] - masterSm) * sm;
            bendSm += (bend - bendSm) * 0.002f; modSm += (mod - modSm) * 0.002f;

            lfoPhase += lfoHz / sr; if (lfoPhase >= 1.f) lfoPhase -= 1.f;
            const float lfo = 4.f * std::fabs(lfoPhase - 0.5f) - 1.f;
            lfoTime += 1.f / sr;
            const float fade = lfoDelaySec < 1e-3f ? 1.f : std::min(1.f, lfoTime / lfoDelaySec);
            const float vib = lfo * (fade * 1.5f * dcoLfo * dcoLfo + modSm * modPitch * 1.0f);
            const float lfoOct = lfo * (fade * lfoVcf * 3.f + modSm * modFilter * 3.f);
            const float k = 4.3f * resSm, lvl = lvlSm * lvlSm * 1.6f * (mode == 1 ? 0.42f : 1.f);   // unison stacks 6 voices on one note

            float mono = 0.f;
            for (auto& v : voices) {
                if (!v.active) continue;
                v.pitch += (v.target - v.pitch) * glideCoef;
                const float semis = v.pitch + bendSm * bendRange + tuneSemis + v.detune * 0.01f + vib;
                const float f = std::min(440.f * std::exp2((semis - 69.f) / 12.f) * rangeMul, 0.45f * sr);
                const float dt = f / sr;
                v.ph += dt; if (v.ph >= 1.f) { v.ph -= 1.f; v.subFlip ^= 1; }
                const float t = v.ph;
                float w = pwmLfo ? 0.5f + 0.45f * pwm * lfo : 0.5f + 0.45f * pwm;
                w = std::max(0.05f, std::min(0.95f, w));
                const float saw = 2.f * t - 1.f - polyBlep(t, dt);
                const float pul = (t < w ? 1.f : -1.f) + polyBlep(t, dt) - polyBlep(std::fmod(t - w + 1.f, 1.f), dt);
                const float st = (v.subFlip ? 0.5f : 0.f) + 0.5f * t, sdt = dt * 0.5f;
                const float sb = (st < 0.5f ? 1.f : -1.f) + polyBlep(st, sdt) - polyBlep(std::fmod(st + 0.5f, 1.f), sdt);
                float x = 0.4f * (sawOn * saw + pulseOn * pul + subLvl * sb + noiseLvl * rng.next());

                if (hpfMode == 0)      { v.hpLp += aHpf * (x - v.hpLp); x = x + 0.7f * v.hpLp; }
                else if (hpfMode >= 2) { v.hpLp += aHpf * (x - v.hpLp); x = x - v.hpLp; }

                const float e = v.env.tick();
                const float oct = cutSm * 10.f + pol * envAmt * 6.f * e + lfoOct + keyFollow * (v.pitch - 60.f) / 12.f;
                const float fc = std::max(16.f, std::min(std::min(20000.f, 0.45f * sr), 20.f * std::exp2(oct) * v.cutVar));
                const float y = v.lad.tick(x, std::tan(kPi * fc / sr), k);

                float amp;
                if (gateMode) {
                    v.vca += ((v.gate ? 1.f : 0.f) - v.vca) * gateCoef; amp = v.vca;
                    if (!v.gate && v.vca < 0.0005f) { v.active = false; v.vca = 0.f; v.lad.reset(); v.hpLp = 0.f; v.env.reset(); }
                } else {
                    amp = e;
                    if (v.env.stage == Adsr::Idle) { v.active = false; v.lad.reset(); v.hpLp = 0.f; }
                }
                mono += y * amp * lvl * v.velGain;
            }

            float l, r; chorus.process(mono, l, r);
            if (satOn) { const float sl = satL.tick(l, satPre, satComp), sr2 = satR.tick(r, satPre, satComp); l += (sl - l) * satMix; r += (sr2 - r) * satMix; }
            if (compOn) comp.process(l, r);
            if (revMix > 0.001f) { float wl, wr; rev.process(l, r, wl, wr); l = l * revDry + wl * revWet; r = r * revDry + wr * revWet; }
            const float mg = masterSm * masterSm * 2.f;
            outL[i] = softLimit(l * mg); outR[i] = softLimit(r * mg);
        }
    }

private:
    // transparent below 0.8 FS, smoothly rounds peaks above it so a loud chord never hard-clips the host
    static float softLimit(float x)
    {
        const float a = std::fabs(x);
        if (a <= 0.8f) return x;
        const float y = 0.8f + 0.2f * std::tanh((a - 0.8f) * 5.f);
        return x < 0.f ? -y : y;
    }
    void trigger(Voice& v, int note, float velGain, float detuneCents)
    {
        const bool was = v.active;
        v.note = note; v.velGain = velGain; v.age = ++ageCounter; v.target = (float)note; v.detune = detuneCents;
        if (!was) { v.pitch = (glideCoef < 1.f) ? lastNote : (float)note; v.lad.reset(); v.hpLp = 0.f; v.vca = 0.f; }
        else if (glideCoef >= 1.f) v.pitch = (float)note;
        v.gate = true; v.sustained = false; v.active = true; v.env.gateOn();
    }
    static void release(Voice& v) { v.gate = false; v.env.gateOff(); }

    float sr = 48000.f, p[P::COUNT] = {};
    Voice voices[kVoices];
    Chorus chorus; Saturator satL, satR; Compressor comp; Reverb rev; Rng rng;
    std::vector<int> stack;
    int mode = 0, hpfMode = 1; bool pedal = false;
    float rangeMul = 1.f, aHpf = 0.f, lfoHz = 5.f, lfoDelaySec = 0.f, glideCoef = 1.f, gateCoef = 0.1f;
    float lfoPhase = 0.f, lfoTime = 1e9f, bend = 0.f, bendSm = 0.f, mod = 0.f, modSm = 0.f, lastNote = 60.f;
    float cutSm = 0.6f, resSm = 0.f, lvlSm = 0.7f, masterSm = 0.7f;
    uint32_t ageCounter = 0;
};

} // namespace juno
