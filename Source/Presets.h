#pragma once
// Juno-106-style factory patches, recreated by ear/by design (NOT a ROM dump of the hardware presets).
// Values are the same 0..1 "slider" positions as the panel. Anything not listed uses the default.
#include <utility>
#include <vector>

namespace juno {

struct Preset {
    const char* name;
    std::vector<std::pair<const char*, float>> v;
};

inline const std::vector<Preset>& presets()
{
    static const std::vector<Preset> list = {
    // ---- Brass
    { "A11 Brass 1",        {{"saw",1},{"pulse",0},{"sub",0.15f},{"hpf",1},{"cutoff",0.38f},{"res",0.1f},{"envAmt",0.5f},{"keyFollow",0.5f},{"attack",0.2f},{"decay",0.45f},{"sustain",0.6f},{"release",0.18f},{"chorus",1}} },
    { "A12 Brass 2",        {{"saw",1},{"pulse",1},{"sub",0.2f},{"hpf",1},{"cutoff",0.3f},{"res",0.15f},{"envAmt",0.6f},{"keyFollow",0.4f},{"attack",0.25f},{"decay",0.5f},{"sustain",0.5f},{"release",0.2f},{"dcoLfo",0.1f},{"lfoRate",0.5f},{"lfoDelay",0.5f},{"chorus",1}} },
    { "A13 Brass Stab",     {{"saw",1},{"pulse",1},{"sub",0.1f},{"cutoff",0.25f},{"res",0.2f},{"envAmt",0.75f},{"attack",0.05f},{"decay",0.35f},{"sustain",0.3f},{"release",0.15f},{"chorus",2}} },
    { "A14 Soft Brass",     {{"saw",1},{"pulse",0},{"sub",0.25f},{"cutoff",0.28f},{"res",0.05f},{"envAmt",0.4f},{"attack",0.35f},{"decay",0.6f},{"sustain",0.7f},{"release",0.3f},{"chorus",1},{"revMix",0.12f}} },
    // ---- Strings & pads
    { "A21 Strings",        {{"vcaLevel",0.6f},{"saw",1},{"pulse",1},{"dcoPwm",0.5f},{"pwmMode",1},{"lfoRate",0.3f},{"hpf",1},{"cutoff",0.6f},{"res",0.1f},{"envAmt",0.1f},{"attack",0.45f},{"decay",0.5f},{"sustain",1.f},{"release",0.5f},{"chorus",2},{"revMix",0.12f}} },
    { "A22 Warm Strings",   {{"saw",1},{"pulse",0},{"sub",0.2f},{"cutoff",0.5f},{"res",0.1f},{"envAmt",0.15f},{"attack",0.5f},{"decay",0.5f},{"sustain",0.9f},{"release",0.55f},{"chorus",1},{"revMix",0.15f}} },
    { "A23 Slow Strings",   {{"saw",1},{"pulse",1},{"dcoPwm",0.4f},{"pwmMode",1},{"lfoRate",0.2f},{"cutoff",0.55f},{"envAmt",0.1f},{"attack",0.7f},{"sustain",1.f},{"release",0.65f},{"chorus",2},{"revMix",0.2f}} },
    { "A24 PWM Pad",        {{"vcaLevel",0.55f},{"saw",0},{"pulse",1},{"dcoPwm",0.7f},{"pwmMode",1},{"lfoRate",0.25f},{"cutoff",0.5f},{"res",0.15f},{"envAmt",0.2f},{"attack",0.55f},{"sustain",0.9f},{"release",0.6f},{"chorus",2},{"revMix",0.2f}} },
    { "A25 Soft Pad",       {{"saw",1},{"pulse",0},{"sub",0.3f},{"cutoff",0.42f},{"res",0.05f},{"envAmt",0.2f},{"attack",0.6f},{"sustain",1.f},{"release",0.7f},{"chorus",1},{"revMix",0.25f}} },
    { "A26 Choir Pad",      {{"saw",0},{"pulse",1},{"dcoPwm",0.3f},{"noise",0.06f},{"cutoff",0.38f},{"res",0.45f},{"envAmt",0.1f},{"keyFollow",0.8f},{"attack",0.55f},{"sustain",1.f},{"release",0.6f},{"chorus",2},{"revMix",0.25f}} },
    { "A27 Space Pad",      {{"saw",1},{"pulse",1},{"dcoPwm",0.6f},{"pwmMode",1},{"lfoRate",0.15f},{"cutoff",0.35f},{"res",0.35f},{"envAmt",0.45f},{"lfoVcf",0.2f},{"attack",0.75f},{"decay",0.8f},{"sustain",0.7f},{"release",0.8f},{"chorus",1},{"revMix",0.35f},{"revSize",0.8f}} },
    { "A28 Sweep Pad",      {{"saw",1},{"pulse",0},{"sub",0.2f},{"cutoff",0.2f},{"res",0.5f},{"envAmt",0.7f},{"attack",0.7f},{"decay",0.75f},{"sustain",0.5f},{"release",0.7f},{"chorus",2},{"revMix",0.25f}} },
    { "A31 Glass Pad",      {{"saw",0},{"pulse",1},{"dcoPwm",0.2f},{"range",2},{"hpf",2},{"cutoff",0.6f},{"res",0.3f},{"envAmt",0.2f},{"attack",0.5f},{"sustain",0.8f},{"release",0.7f},{"chorus",3},{"revMix",0.3f}} },
    { "A32 Evolving Pad",   {{"saw",1},{"pulse",1},{"dcoPwm",0.8f},{"pwmMode",1},{"lfoRate",0.1f},{"cutoff",0.4f},{"res",0.3f},{"lfoVcf",0.3f},{"envAmt",0.3f},{"attack",0.8f},{"decay",0.8f},{"sustain",0.8f},{"release",0.85f},{"chorus",2},{"revMix",0.35f},{"revSize",0.85f}} },
    // ---- Bass
    { "B11 Bass 1",         {{"range",0},{"saw",1},{"pulse",0},{"sub",0.5f},{"cutoff",0.28f},{"res",0.2f},{"envAmt",0.45f},{"keyFollow",0.3f},{"attack",0.0f},{"decay",0.35f},{"sustain",0.3f},{"release",0.1f}} },
    { "B12 Fat Bass",       {{"vcaLevel",0.6f},{"range",0},{"saw",1},{"pulse",1},{"sub",0.7f},{"cutoff",0.3f},{"res",0.15f},{"envAmt",0.35f},{"decay",0.4f},{"sustain",0.5f},{"release",0.1f},{"chorus",1}} },
    { "B13 Sub Bass",       {{"vcaLevel",0.55f},{"range",0},{"saw",0},{"pulse",0},{"sub",1.f},{"cutoff",0.35f},{"res",0.0f},{"envAmt",0.1f},{"keyFollow",0.0f},{"attack",0.0f},{"decay",0.4f},{"sustain",0.9f},{"release",0.12f}} },
    { "B14 Resonant Bass",  {{"vcaLevel",1.0f},{"range",0},{"saw",1},{"pulse",0},{"sub",0.4f},{"cutoff",0.2f},{"res",0.65f},{"envAmt",0.6f},{"keyFollow",0.4f},{"decay",0.3f},{"sustain",0.2f},{"release",0.1f}} },
    { "B15 Pluck Bass",     {{"vcaLevel",0.6f},{"range",0},{"saw",0},{"pulse",1},{"dcoPwm",0.2f},{"sub",0.5f},{"cutoff",0.22f},{"res",0.3f},{"envAmt",0.65f},{"decay",0.25f},{"sustain",0.0f},{"release",0.12f}} },
    { "B16 Wobble Bass",    {{"range",0},{"saw",1},{"pulse",1},{"sub",0.5f},{"cutoff",0.3f},{"res",0.5f},{"lfoVcf",0.6f},{"lfoRate",0.6f},{"envAmt",0.2f},{"sustain",0.8f},{"release",0.12f}} },
    { "B17 Acid Bass",      {{"range",0},{"saw",1},{"pulse",0},{"cutoff",0.18f},{"res",0.75f},{"envAmt",0.7f},{"decay",0.28f},{"sustain",0.0f},{"release",0.1f},{"porta",0.25f},{"voiceMode",1}} },
    // ---- Leads
    { "B21 Lead 1",         {{"saw",1},{"pulse",0},{"sub",0.1f},{"cutoff",0.55f},{"res",0.25f},{"envAmt",0.25f},{"keyFollow",0.8f},{"attack",0.0f},{"sustain",0.9f},{"release",0.15f},{"dcoLfo",0.12f},{"lfoRate",0.5f},{"lfoDelay",0.4f},{"voiceMode",1},{"porta",0.2f},{"chorus",1}} },
    { "B22 Soft Lead",      {{"saw",0},{"pulse",1},{"dcoPwm",0.3f},{"cutoff",0.45f},{"res",0.15f},{"envAmt",0.2f},{"keyFollow",0.7f},{"attack",0.05f},{"sustain",0.9f},{"release",0.2f},{"voiceMode",1},{"chorus",2},{"revMix",0.15f}} },
    { "B23 PWM Lead",       {{"saw",0},{"pulse",1},{"dcoPwm",0.6f},{"pwmMode",1},{"lfoRate",0.4f},{"cutoff",0.55f},{"res",0.2f},{"keyFollow",0.8f},{"sustain",0.9f},{"release",0.2f},{"voiceMode",1},{"chorus",1}} },
    { "B24 Vibrato Lead",   {{"saw",1},{"pulse",1},{"cutoff",0.5f},{"res",0.3f},{"envAmt",0.3f},{"keyFollow",0.7f},{"attack",0.1f},{"sustain",0.8f},{"release",0.25f},{"dcoLfo",0.2f},{"lfoRate",0.55f},{"lfoDelay",0.6f},{"voiceMode",1},{"porta",0.15f},{"chorus",1}} },
    { "B25 Whistle",        {{"vcaLevel",1.0f},{"saw",0},{"pulse",0},{"sub",0.0f},{"noise",0.04f},{"range",2},{"cutoff",0.55f},{"res",0.97f},{"keyFollow",1.f},{"envAmt",0.0f},{"attack",0.1f},{"sustain",1.f},{"release",0.2f},{"dcoLfo",0.1f},{"lfoDelay",0.6f},{"voiceMode",1},{"chorus",1}} },
    { "B26 Mono Saw Lead",  {{"saw",1},{"pulse",0},{"sub",0.2f},{"cutoff",0.6f},{"res",0.2f},{"envAmt",0.3f},{"keyFollow",0.7f},{"sustain",0.9f},{"release",0.1f},{"voiceMode",1},{"porta",0.3f},{"satDrive",0.3f},{"revMix",0.1f}} },
    // ---- Keys / plucks
    { "B31 Piano",          {{"saw",0},{"pulse",1},{"dcoPwm",0.15f},{"hpf",2},{"cutoff",0.45f},{"res",0.05f},{"envAmt",0.45f},{"keyFollow",0.6f},{"attack",0.0f},{"decay",0.55f},{"sustain",0.0f},{"release",0.35f},{"chorus",1},{"velocity",0.5f}} },
    { "B32 Electric Piano", {{"saw",0},{"pulse",1},{"dcoPwm",0.3f},{"sub",0.2f},{"hpf",2},{"cutoff",0.4f},{"res",0.1f},{"envAmt",0.35f},{"keyFollow",0.6f},{"decay",0.5f},{"sustain",0.05f},{"release",0.35f},{"chorus",2},{"velocity",0.4f}} },
    { "B33 Clav",           {{"vcaLevel",1.0f},{"saw",1},{"pulse",1},{"dcoPwm",0.4f},{"hpf",2},{"cutoff",0.35f},{"res",0.25f},{"envAmt",0.5f},{"keyFollow",0.7f},{"decay",0.25f},{"sustain",0.0f},{"release",0.15f}} },
    { "B34 Harpsichord",    {{"vcaLevel",1.0f},{"saw",1},{"pulse",1},{"dcoPwm",0.1f},{"hpf",3},{"cutoff",0.5f},{"res",0.15f},{"envAmt",0.5f},{"keyFollow",0.8f},{"decay",0.3f},{"sustain",0.0f},{"release",0.2f},{"chorus",1}} },
    { "B35 Marimba",        {{"saw",0},{"pulse",1},{"dcoPwm",0.0f},{"hpf",2},{"cutoff",0.3f},{"res",0.2f},{"envAmt",0.45f},{"keyFollow",0.8f},{"decay",0.28f},{"sustain",0.0f},{"release",0.2f},{"revMix",0.12f}} },
    { "B36 Harp Pluck",     {{"saw",0},{"pulse",1},{"dcoPwm",0.25f},{"hpf",2},{"cutoff",0.42f},{"res",0.1f},{"envAmt",0.4f},{"keyFollow",0.7f},{"decay",0.45f},{"sustain",0.0f},{"release",0.4f},{"chorus",2},{"revMix",0.25f}} },
    { "B37 Bell",           {{"saw",0},{"pulse",1},{"range",2},{"hpf",3},{"cutoff",0.55f},{"res",0.45f},{"envAmt",0.3f},{"keyFollow",1.f},{"decay",0.6f},{"sustain",0.0f},{"release",0.55f},{"chorus",3},{"revMix",0.3f}} },
    { "B38 Organ",          {{"vcaLevel",0.5f},{"saw",0},{"pulse",1},{"sub",0.7f},{"hpf",1},{"cutoff",0.7f},{"res",0.0f},{"envAmt",0.0f},{"attack",0.0f},{"sustain",1.f},{"release",0.05f},{"vcaMode",1},{"chorus",2}} },
    { "B41 Flute",          {{"saw",0},{"pulse",1},{"dcoPwm",0.0f},{"noise",0.08f},{"range",2},{"hpf",2},{"cutoff",0.4f},{"res",0.2f},{"envAmt",0.15f},{"keyFollow",0.8f},{"attack",0.2f},{"sustain",0.9f},{"release",0.2f},{"dcoLfo",0.1f},{"lfoDelay",0.7f},{"lfoRate",0.5f},{"chorus",1},{"revMix",0.15f}} },
    // ---- FX / misc
    { "B51 Noise Sweep",    {{"saw",0},{"pulse",0},{"noise",1.f},{"cutoff",0.25f},{"res",0.55f},{"envAmt",0.8f},{"attack",0.7f},{"decay",0.7f},{"sustain",0.0f},{"release",0.6f},{"chorus",2},{"revMix",0.3f}} },
    { "B52 Wind",           {{"vcaLevel",1.0f},{"saw",0},{"pulse",0},{"noise",0.9f},{"cutoff",0.35f},{"res",0.4f},{"lfoVcf",0.35f},{"lfoRate",0.15f},{"envAmt",0.2f},{"attack",0.7f},{"sustain",1.f},{"release",0.7f},{"chorus",1},{"revMix",0.3f}} },
    { "B53 Laser",          {{"saw",1},{"pulse",0},{"cutoff",0.8f},{"res",0.6f},{"envAmt",0.9f},{"vcfPol",1},{"decay",0.25f},{"sustain",0.0f},{"release",0.15f},{"dcoLfo",0.9f},{"lfoRate",0.9f},{"lfoDelay",0.0f}} },
    { "B54 Chime Arp",      {{"saw",0},{"pulse",1},{"range",2},{"hpf",2},{"cutoff",0.5f},{"res",0.3f},{"envAmt",0.4f},{"decay",0.35f},{"sustain",0.0f},{"release",0.3f},{"chorus",3},{"revMix",0.3f}} },
    { "B55 Detuned Stack",  {{"saw",1},{"pulse",1},{"sub",0.3f},{"cutoff",0.5f},{"res",0.2f},{"envAmt",0.3f},{"attack",0.05f},{"sustain",0.8f},{"release",0.3f},{"voiceMode",1},{"chorus",3},{"satDrive",0.25f},{"revMix",0.15f}} },
    };
    return list;
}

} // namespace juno
