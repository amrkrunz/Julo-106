#pragma once
// Parameter table shared by the DSP engine, presets, plugin wrapper and editor.
namespace juno {

enum class Kind { Float, Choice, Bool };

struct ParamDef {
    const char* id;
    const char* name;
    Kind kind;
    float mn, mx, def, step;   // for Choice: mn=0, mx=numChoices-1; for Bool: 0..1
    const char* choices;       // '|' separated, Choice only
};

constexpr Kind kF = Kind::Float;
constexpr Kind kC = Kind::Choice;
constexpr Kind kB = Kind::Bool;

//   id          display name      kind   min    max    def     step  choices
#define J_PARAM_LIST(X) \
 X(lfoRate,    "LFO Rate",       kF,   0.f,   1.f,   0.35f,  0.f, "") \
 X(lfoDelay,   "LFO Delay",      kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(dcoLfo,     "DCO LFO",        kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(dcoPwm,     "PWM",            kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(sub,        "Sub",            kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(noise,      "Noise",          kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(hpf,        "HPF",            kF,   0.f,   3.f,   1.0f,   1.f, "") \
 X(cutoff,     "Cutoff",         kF,   0.f,   1.f,   0.65f,  0.f, "") \
 X(res,        "Resonance",      kF,   0.f,   1.f,   0.2f,   0.f, "") \
 X(envAmt,     "Env Amount",     kF,   0.f,   1.f,   0.3f,   0.f, "") \
 X(lfoVcf,     "LFO > VCF",      kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(keyFollow,  "Key Follow",     kF,   0.f,   1.f,   0.5f,   0.f, "") \
 X(vcaLevel,   "VCA Level",      kF,   0.f,   1.f,   0.75f,  0.f, "") \
 X(attack,     "Attack",         kF,   0.f,   1.f,   0.02f,  0.f, "") \
 X(decay,      "Decay",          kF,   0.f,   1.f,   0.45f,  0.f, "") \
 X(sustain,    "Sustain",        kF,   0.f,   1.f,   0.6f,   0.f, "") \
 X(release,    "Release",        kF,   0.f,   1.f,   0.3f,   0.f, "") \
 X(range,      "Range",          kC,   0.f,   2.f,   1.0f,   1.f, "16'|8'|4'") \
 X(saw,        "Saw",            kB,   0.f,   1.f,   1.0f,   1.f, "") \
 X(pulse,      "Pulse",          kB,   0.f,   1.f,   1.0f,   1.f, "") \
 X(pwmMode,    "PWM Mode",       kC,   0.f,   1.f,   0.0f,   1.f, "Manual|LFO") \
 X(vcfPol,     "Env Polarity",   kC,   0.f,   1.f,   0.0f,   1.f, "Normal|Inverted") \
 X(vcaMode,    "VCA Mode",       kC,   0.f,   1.f,   0.0f,   1.f, "Env|Gate") \
 X(chorus,     "Chorus",         kC,   0.f,   3.f,   0.0f,   1.f, "Off|I|II|I+II") \
 X(voiceMode,  "Voice Mode",     kC,   0.f,   1.f,   0.0f,   1.f, "Poly|Unison") \
 X(porta,      "Portamento",     kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(bendRange,  "Bend Range",     kF,   1.f,   12.f,  2.0f,   1.f, "") \
 X(modPitch,   "Mod > Pitch",    kF,   0.f,   1.f,   1.0f,   0.f, "") \
 X(modFilter,  "Mod > Filter",   kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(velocity,   "Velocity",       kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(tune,       "Tune",           kF,  -100.f, 100.f, 0.0f,   0.f, "") \
 X(satDrive,   "Sat Drive",      kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(satMix,     "Sat Mix",        kF,   0.f,   1.f,   1.0f,   0.f, "") \
 X(compOn,     "Comp On",        kB,   0.f,   1.f,   0.0f,   1.f, "") \
 X(compThresh, "Comp Threshold", kF,  -40.f,  0.f,  -18.0f,  0.f, "") \
 X(compRatio,  "Comp Ratio",     kF,   1.f,   20.f,  4.0f,   0.f, "") \
 X(compAttack, "Comp Attack",    kF,   0.1f,  100.f, 10.0f,  0.f, "") \
 X(compRelease,"Comp Release",   kF,   10.f,  1000.f,150.0f, 0.f, "") \
 X(compMakeup, "Comp Makeup",    kF,   0.f,   18.f,  0.0f,   0.f, "") \
 X(revMix,     "Reverb Mix",     kF,   0.f,   1.f,   0.0f,   0.f, "") \
 X(revSize,    "Reverb Size",    kF,   0.f,   1.f,   0.5f,   0.f, "") \
 X(revDamp,    "Reverb Damp",    kF,   0.f,   1.f,   0.5f,   0.f, "") \
 X(master,     "Master",         kF,   0.f,   1.f,   0.7f,   0.f, "")

namespace P {
enum {
#define X(id, ...) id,
    J_PARAM_LIST(X)
#undef X
    COUNT
};
}

inline const ParamDef* paramDefs()
{
    static const ParamDef d[] = {
#define X(id, name, kind, mn, mx, df, step, ch) { #id, name, kind, mn, mx, df, step, ch },
        J_PARAM_LIST(X)
#undef X
    };
    return d;
}

} // namespace juno
