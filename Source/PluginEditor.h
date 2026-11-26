#pragma once

#include "PluginProcessor.h"

// Base (unscaled) editor size. The window can be resized with a fixed aspect ratio.
constexpr int kFaceW = 560, kFaceH = 330;

struct Theme
{
    juce::Colour bg, border, panel, panelBorder, accent, hi, text, dim, padBg, grid,
                 btnTop, btnBot, btnText, btnOnText, whiteKey, blackKey, keyLine, keyDown;
};
const Theme& themeFor (int mode);
juce::Font themeFont (int mode, float height);

//==============================================================================
class ThemeLnF : public juce::LookAndFeel_V4
{
public:
    int mode = 0;

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
};

//==============================================================================
// XY pad: X = vowel (U O A E I), Y = pitch. Press to sing.
class ChantPad : public juce::Component, private juce::Timer
{
public:
    explicit ChantPad (MonkMachineProcessor&);
    ~ChantPad() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void updateFromMouse (juce::Point<float> pos);
    void releaseGate();
    void drawMouth (juce::Graphics&, int mode, juce::Point<float> c, float mw, float mh, float env);

    MonkMachineProcessor& proc;
    juce::RangedAudioParameter* vowel;
    juce::RangedAudioParameter* pitch;
    juce::RangedAudioParameter* gate;
    bool dragging = false;
    int frame = 0;
    juce::Random rng;
    struct Fog { float x, y, r, s; };
    std::vector<Fog> fog;
};

//==============================================================================
class Face : public juce::Component, private juce::Timer
{
public:
    Face (MonkMachineProcessor&, ThemeLnF&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void addKnob (const juce::String& id, const juce::String& name, bool voiceSection);
    void applyTheme();
    void timerCallback() override;
    void paintBackground (juce::Graphics&, const Theme&);
    void paintPlate (juce::Graphics&, const Theme&, juce::Rectangle<float>);

    MonkMachineProcessor& proc;
    ThemeLnF& lnf;
    ChantPad pad;
    juce::TextButton modeButtons[3];
    juce::TextButton autoButton { "Auto voice" };
    juce::MidiKeyboardComponent keyboard;
    std::vector<std::unique_ptr<Knob>> voiceKnobs, spaceKnobs;
    int shownMode = -1;
    juce::Rectangle<float> titleArea, plateArea, panelArea, ledArea;
};

//==============================================================================
class MonkMachineEditor : public juce::AudioProcessorEditor
{
public:
    explicit MonkMachineEditor (MonkMachineProcessor&);
    ~MonkMachineEditor() override;
    void resized() override;

private:
    ThemeLnF lnf;   // must outlive face
    Face face;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonkMachineEditor)
};
