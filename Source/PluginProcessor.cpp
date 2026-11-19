#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstring>

using namespace juno;

juce::AudioProcessorValueTreeState::ParameterLayout J106Processor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> ps;
    const ParamDef* d = paramDefs();
    for (int i = 0; i < P::COUNT; ++i)
    {
        const juce::ParameterID pid { d[i].id, 1 };
        switch (d[i].kind)
        {
            case Kind::Float:
            {
                juce::NormalisableRange<float> range(d[i].mn, d[i].mx);
                if (d[i].step > 0.f) range.interval = d[i].step;
                ps.push_back(std::make_unique<juce::AudioParameterFloat>(pid, d[i].name, range, d[i].def));
                break;
            }
            case Kind::Choice:
            {
                juce::StringArray items; items.addTokens(d[i].choices, "|", "");
                ps.push_back(std::make_unique<juce::AudioParameterChoice>(pid, d[i].name, items, (int) d[i].def));
                break;
            }
            case Kind::Bool:
                ps.push_back(std::make_unique<juce::AudioParameterBool>(pid, d[i].name, d[i].def > 0.5f));
                break;
        }
    }
    return { ps.begin(), ps.end() };
}

J106Processor::J106Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "J106", createLayout())
{
    for (int i = 0; i < P::COUNT; ++i)
        raw[i] = apvts.getRawParameterValue(paramDefs()[i].id);
    engine.prepare(44100.0);   // safe defaults in case a host calls processBlock before prepareToPlay
}

bool J106Processor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void J106Processor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
}

void J106Processor::handleMidi(const juce::MidiMessage& m)
{
    if (m.isNoteOn())                       engine.noteOn(m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())                 engine.noteOff(m.getNoteNumber());
    else if (m.isPitchWheel())              engine.pitchBend((float) (m.getPitchWheelValue() - 8192) / 8192.f);
    else if (m.isController())
    {
        const int cc = m.getControllerNumber(), v = m.getControllerValue();
        if (cc == 1)                        engine.modWheel((float) v / 127.f);
        else if (cc == 64)                  engine.sustainPedal(v >= 64);
        else if (cc == 120 || cc == 123)    engine.allNotesOff();
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
}

void J106Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();

    float values[P::COUNT];
    for (int i = 0; i < P::COUNT; ++i) values[i] = raw[i]->load();
    engine.setParams(values);

    keyboardState.processNextMidiBuffer(midi, 0, n, true);   // notes from the on-screen keyboard

    const float ub = uiBend.load(), um = uiMod.load();
    if (ub != lastUiBend) { engine.pitchBend(ub); lastUiBend = ub; }
    if (um != lastUiMod)  { engine.modWheel(um);  lastUiMod  = um; }

    float* left = buffer.getWritePointer(0);
    juce::HeapBlock<float> scratch;
    float* right = nullptr;
    if (buffer.getNumChannels() > 1) right = buffer.getWritePointer(1);
    else { scratch.calloc((size_t) n); right = scratch.get(); }

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit(0, n, meta.samplePosition);
        if (t > pos) { engine.process(left + pos, right + pos, t - pos); pos = t; }
        handleMidi(meta.getMessage());
    }
    if (pos < n) engine.process(left + pos, right + pos, n - pos);

    if (buffer.getNumChannels() == 1)      // mono bus: fold stereo down
        for (int i = 0; i < n; ++i) left[i] = 0.5f * (left[i] + right[i]);
    midi.clear();
}

const juce::String J106Processor::getProgramName(int index)
{
    const auto& list = presets();
    return (index >= 0 && index < (int) list.size()) ? juce::String(list[(size_t) index].name) : juce::String();
}

void J106Processor::setCurrentProgram(int index)
{
    const auto& list = presets();
    if (index < 0 || index >= (int) list.size()) return;
    if (index == currentProgram) return;   // hosts re-send the current program on load; don't overwrite saved/tweaked settings
    currentProgram = index;

    const ParamDef* d = paramDefs();
    float v[P::COUNT];
    for (int i = 0; i < P::COUNT; ++i) v[i] = d[i].def;
    // Patches only describe the synth section; keep the user's FX / performance settings unless the patch sets them.
    for (int i = P::tune; i < P::COUNT; ++i) v[i] = raw[i]->load();
    for (const auto& kv : list[(size_t) index].v)
        for (int i = 0; i < P::COUNT; ++i)
            if (std::strcmp(d[i].id, kv.first) == 0) v[i] = kv.second;

    for (int i = 0; i < P::COUNT; ++i)
        if (auto* prm = apvts.getParameter(d[i].id))
            prm->setValueNotifyingHost(prm->convertTo0to1(v[i]));
}

void J106Processor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty("program", currentProgram, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest);
}

void J106Processor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            currentProgram = juce::jlimit(0, (int) presets().size() - 1, (int) tree.getProperty("program", 0));
            apvts.replaceState(tree);
        }
}

juce::AudioProcessorEditor* J106Processor::createEditor() { return new J106Editor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new J106Processor(); }
