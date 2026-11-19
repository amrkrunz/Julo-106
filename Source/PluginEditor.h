#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <vector>

class J106Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit J106Editor(J106Processor&);
    ~J106Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Control
    {
        int index = 0;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::ComboBox> combo;
        std::unique_ptr<juce::ToggleButton> toggle;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sa;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ca;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ba;
        juce::Component* comp() const { return slider ? (juce::Component*) slider.get() : combo ? (juce::Component*) combo.get() : (juce::Component*) toggle.get(); }
    };

    struct Group { juce::String title; juce::Colour colour; std::vector<int> params; juce::Rectangle<int> bounds; bool vertical; };

    void addControl(int paramIndex, bool vertical, const juce::String& labelText);
    Control* find(int paramIndex);
    void layoutGroup(Group& g, juce::Rectangle<int> area);
    void timerCallback() override;
    void syncPresetBox();

    J106Processor& proc;
    std::vector<std::unique_ptr<Control>> controls;
    std::vector<Group> groups;

    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" };
    juce::Slider bendWheel, modWheel;
    juce::Label bendLabel, modLabel;
    juce::MidiKeyboardComponent keyboard;
    int shownProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(J106Editor)
};
