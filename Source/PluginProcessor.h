#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "Presets.h"

class J106Processor : public juce::AudioProcessor
{
public:
    J106Processor();
    ~J106Processor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "J-106"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override { return (int) juno::presets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;     // on-screen keyboard
    std::atomic<float> uiBend { 0.f }, uiMod { 0.f };   // on-screen wheels

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleMidi(const juce::MidiMessage& m);

    juno::Engine engine;
    std::atomic<float>* raw[juno::P::COUNT] = {};
    int currentProgram = 0;
    float lastUiBend = 0.f, lastUiMod = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J106Processor)
};
