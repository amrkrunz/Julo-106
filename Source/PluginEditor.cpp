#include "PluginEditor.h"

using namespace juno;

namespace
{
    const juce::Colour kPanel  (0xff23262b);
    const juce::Colour kBox    (0xff2d3138);
    const juce::Colour kText   (0xffe6e6e6);
    const juce::Colour kOrange (0xffe8833a), kRed(0xffd9453d), kYellow(0xffe5c04a), kBlue(0xff4a90d9),
                       kGreen  (0xff55b36b), kPurple(0xff9a6bd6), kGrey(0xff9aa3ad);
}

J106Editor::J106Editor(J106Processor& p)
    : AudioProcessorEditor(&p), proc(p),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // ---- panel sliders (same order and grouping as the hardware panel)
    const struct { const char* title; juce::Colour col; std::vector<int> ids; } panel[] = {
        { "LFO",  kOrange, { P::lfoRate, P::lfoDelay } },
        { "DCO",  kOrange, { P::dcoLfo, P::dcoPwm, P::sub, P::noise } },
        { "HPF",  kYellow, { P::hpf } },
        { "VCF",  kRed,    { P::cutoff, P::res, P::envAmt, P::lfoVcf, P::keyFollow } },
        { "VCA",  kBlue,   { P::vcaLevel } },
        { "ENV",  kGreen,  { P::attack, P::decay, P::sustain, P::release } },
    };
    const char* shortNames[] = { "Rate", "Delay", "LFO", "PWM", "Sub", "Noise", "HPF", "Freq", "Res", "Env", "LFO", "Kybd", "Level", "A", "D", "S", "R" };
    int sn = 0;
    for (auto& g : panel)
    {
        groups.push_back({ g.title, g.col, g.ids, {}, true });
        for (int id : g.ids) addControl(id, true, shortNames[sn++]);
    }

    // ---- switches / performance
    groups.push_back({ "OSCILLATOR", kOrange, { P::range, P::saw, P::pulse, P::pwmMode }, {}, false });
    groups.push_back({ "FILTER / AMP", kRed,  { P::vcfPol, P::vcaMode }, {}, false });
    groups.push_back({ "CHORUS / VOICES", kPurple, { P::chorus, P::voiceMode }, {}, false });
    groups.push_back({ "PERFORMANCE", kYellow, { P::porta, P::bendRange, P::modPitch, P::modFilter, P::velocity, P::tune }, {}, false });
    groups.push_back({ "SATURATION", kRed, { P::satDrive, P::satMix }, {}, false });
    groups.push_back({ "COMPRESSOR", kBlue, { P::compOn, P::compThresh, P::compRatio, P::compAttack, P::compRelease, P::compMakeup }, {}, false });
    groups.push_back({ "REVERB", kGreen, { P::revMix, P::revSize, P::revDamp }, {}, false });
    groups.push_back({ "OUT", kGrey, { P::master }, {}, false });
    for (size_t gi = 6; gi < groups.size(); ++gi)
        for (int id : groups[gi].params) addControl(id, false, paramDefs()[id].name);

    // ---- presets
    for (int i = 0; i < proc.getNumPrograms(); ++i) presetBox.addItem(proc.getProgramName(i), i + 1);
    presetBox.onChange = [this] { const int i = presetBox.getSelectedId() - 1; if (i >= 0 && i != proc.getCurrentProgram()) proc.setCurrentProgram(i); };
    addAndMakeVisible(presetBox);
    prevButton.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram((proc.getCurrentProgram() + n - 1) % n); syncPresetBox(); };
    nextButton.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram((proc.getCurrentProgram() + 1) % n); syncPresetBox(); };
    addAndMakeVisible(prevButton); addAndMakeVisible(nextButton);

    // ---- on-screen wheels
    bendWheel.setSliderStyle(juce::Slider::LinearVertical); bendWheel.setRange(-1.0, 1.0, 0.0); bendWheel.setValue(0.0);
    bendWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); bendWheel.setDoubleClickReturnValue(true, 0.0);
    bendWheel.onValueChange = [this] { proc.uiBend.store((float) bendWheel.getValue()); };
    bendWheel.onDragEnd = [this] { bendWheel.setValue(0.0, juce::sendNotificationSync); };   // spring back like a real pitch wheel
    modWheel.setSliderStyle(juce::Slider::LinearVertical); modWheel.setRange(0.0, 1.0, 0.0); modWheel.setValue(0.0);
    modWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    modWheel.onValueChange = [this] { proc.uiMod.store((float) modWheel.getValue()); };
    for (auto* s : { &bendWheel, &modWheel }) { s->setColour(juce::Slider::trackColourId, kOrange); s->setColour(juce::Slider::thumbColourId, kText); addAndMakeVisible(s); }
    bendLabel.setText("PITCH", juce::dontSendNotification); modLabel.setText("MOD", juce::dontSendNotification);
    for (auto* l : { &bendLabel, &modLabel }) { l->setJustificationType(juce::Justification::centred); l->setColour(juce::Label::textColourId, kText); addAndMakeVisible(l); }

    keyboard.setAvailableRange(36, 96);
    keyboard.setLowestVisibleKey(36);
    addAndMakeVisible(keyboard);

    syncPresetBox();
    startTimerHz(8);
    setSize(1180, 700);   // must come last: it triggers resized(), which needs every control to exist
}

J106Editor::~J106Editor() { stopTimer(); }

void J106Editor::addControl(int idx, bool vertical, const juce::String& labelText)
{
    const ParamDef& d = paramDefs()[idx];
    auto c = std::make_unique<Control>();
    c->index = idx;
    if (d.kind == Kind::Float)
    {
        c->slider = std::make_unique<juce::Slider>();
        c->slider->setSliderStyle(vertical ? juce::Slider::LinearVertical : juce::Slider::RotaryVerticalDrag);
        c->slider->setTextBoxStyle(vertical ? juce::Slider::NoTextBox : juce::Slider::TextBoxBelow, false, 56, 16);
        c->slider->setColour(juce::Slider::trackColourId, kOrange);
        c->slider->setColour(juce::Slider::rotarySliderFillColourId, kOrange);
        c->slider->setColour(juce::Slider::thumbColourId, kText);
        c->slider->setColour(juce::Slider::textBoxTextColourId, kText);
        c->slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        c->slider->setDoubleClickReturnValue(true, d.def);
        c->sa = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, d.id, *c->slider);
    }
    else if (d.kind == Kind::Choice)
    {
        c->combo = std::make_unique<juce::ComboBox>();
        juce::StringArray items; items.addTokens(d.choices, "|", "");
        c->combo->addItemList(items, 1);
        c->ca = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, d.id, *c->combo);
    }
    else
    {
        c->toggle = std::make_unique<juce::ToggleButton>(labelText);
        c->toggle->setColour(juce::ToggleButton::textColourId, kText);
        c->ba = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, d.id, *c->toggle);
    }
    if (!c->toggle)
    {
        c->label = std::make_unique<juce::Label>();
        c->label->setText(labelText, juce::dontSendNotification);
        c->label->setJustificationType(juce::Justification::centred);
        c->label->setColour(juce::Label::textColourId, kText);
        addAndMakeVisible(*c->label);
    }
    addAndMakeVisible(*c->comp());
    controls.push_back(std::move(c));
}

J106Editor::Control* J106Editor::find(int idx)
{
    for (auto& c : controls) if (c->index == idx) return c.get();
    return nullptr;
}

void J106Editor::syncPresetBox()
{
    shownProgram = proc.getCurrentProgram();
    presetBox.setSelectedId(shownProgram + 1, juce::dontSendNotification);
}

void J106Editor::timerCallback()
{
    if (proc.getCurrentProgram() != shownProgram) syncPresetBox();
}

void J106Editor::layoutGroup(Group& g, juce::Rectangle<int> area)
{
    g.bounds = area;
    auto inner = area.reduced(6, 4);
    inner.removeFromTop(18);                       // title strip
    const int n = (int) g.params.size();
    if (n == 0) return;
    const int cellW = inner.getWidth() / n;
    for (int i = 0; i < n; ++i)
    {
        auto cell = inner.removeFromLeft(cellW);
        if (auto* c = find(g.params[(size_t) i]))
        {
            if (c->toggle) { c->toggle->setBounds(cell.withSizeKeepingCentre(juce::jmin(cell.getWidth(), 110), 24)); continue; }
            auto lab = cell.removeFromBottom(18);
            c->label->setBounds(lab);
            if (c->combo)  c->combo->setBounds(cell.withSizeKeepingCentre(juce::jmin(cell.getWidth() - 6, 100), 24));
            else           c->slider->setBounds(g.vertical ? cell.reduced(juce::jmax(0, (cell.getWidth() - 36) / 2), 2) : cell.reduced(2));
        }
    }
}

void J106Editor::resized()
{
    if (groups.size() < 14) return;
    auto r = getLocalBounds().reduced(10);
    auto top = r.removeFromTop(34);
    top.removeFromLeft(220);
    prevButton.setBounds(top.removeFromLeft(34));
    presetBox.setBounds(top.removeFromLeft(320).reduced(4, 0));
    nextButton.setBounds(top.removeFromLeft(34));

    // row 1: panel sliders
    auto row1 = r.removeFromTop(250);
    const int totalCells = 2 + 4 + 1 + 5 + 1 + 4;
    const int unit = row1.getWidth() / totalCells;
    for (size_t gi = 0; gi < 6; ++gi)
        layoutGroup(groups[gi], row1.removeFromLeft(unit * (int) groups[gi].params.size()));

    // row 2: switches + performance
    r.removeFromTop(6);
    auto row2 = r.removeFromTop(92);
    const int w2[] = { 4, 2, 2, 6 };
    int sum2 = 0; for (int w : w2) sum2 += w;
    const int total2 = row2.getWidth();
    for (size_t gi = 6; gi < 10; ++gi) layoutGroup(groups[gi], row2.removeFromLeft(gi == 9 ? row2.getWidth() : total2 * w2[gi - 6] / sum2));

    // row 3: FX
    r.removeFromTop(6);
    auto row3 = r.removeFromTop(150);
    const int w3[] = { 2, 6, 3, 1 };
    int sum3 = 0; for (int w : w3) sum3 += w;
    const int total3 = row3.getWidth();
    for (size_t gi = 10; gi < groups.size(); ++gi)
    {
        const int wd = (gi == groups.size() - 1) ? row3.getWidth() : total3 * w3[gi - 10] / sum3;
        layoutGroup(groups[gi], row3.removeFromLeft(wd));
    }

    // keyboard row
    r.removeFromTop(8);
    auto kb = r;
    auto wheels = kb.removeFromLeft(110);
    auto pw = wheels.removeFromLeft(55), mw = wheels;
    bendLabel.setBounds(pw.removeFromBottom(16)); modLabel.setBounds(mw.removeFromBottom(16));
    bendWheel.setBounds(pw.reduced(8, 0)); modWheel.setBounds(mw.reduced(8, 0));
    keyboard.setBounds(kb);
}

void J106Editor::paint(juce::Graphics& g)
{
    g.fillAll(kPanel);
    g.setColour(kText);
    g.setFont(26.0f);
    g.drawText("J-106", 20, 10, 140, 34, juce::Justification::centredLeft);
    g.setFont(12.0f);
    g.setColour(kGrey);
    g.drawText("POLYPHONIC SYNTHESIZER", 20, 38, 200, 14, juce::Justification::centredLeft);

    for (auto& grp : groups)
    {
        g.setColour(kBox);
        g.fillRoundedRectangle(grp.bounds.toFloat().reduced(2.f), 5.f);
        g.setColour(grp.colour);
        g.fillRect(grp.bounds.getX() + 8, grp.bounds.getY() + 22, grp.bounds.getWidth() - 16, 2);
        g.setColour(kText);
        g.setFont(12.0f);
        g.drawText(grp.title, grp.bounds.getX() + 8, grp.bounds.getY() + 5, grp.bounds.getWidth() - 16, 16, juce::Justification::centredLeft);
    }
}
