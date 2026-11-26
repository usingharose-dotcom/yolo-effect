#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "MonkEngine.h"

class MonkMachineProcessor : public juce::AudioProcessor
{
public:
    MonkMachineProcessor();
    ~MonkMachineProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                     { return true; }

    const juce::String getName() const override         { return JucePlugin_Name; }
    bool acceptsMidi() const override                   { return true; }
    bool producesMidi() const override                  { return false; }
    bool isMidiEffect() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 8.0; }

    int getNumPrograms() override                       { return 1; }
    int getCurrentProgram() override                    { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // Message thread: switch mode and load that mode's knob preset
    void applyMode (int mode);
    // Any thread: ask the engine to perform a phrase
    void triggerAutoVoice() { autoRequest.store (true); }
    int  getMode() const    { return (int) pMode->load(); }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    // read by the editor for display
    std::atomic<float> displayVowel { 0.5f }, displayPitch { 0.5f }, displayEnv { 0.f };

private:
    void pullParams();
    void handleMidi (const juce::MidiMessage& m);

    monk::MonkEngine engine;
    monk::Params params;
    std::atomic<bool> autoRequest { false };

    std::atomic<float>* pMode = nullptr;
    std::atomic<float>* pVowel = nullptr;     std::atomic<float>* pPadPitch = nullptr;
    std::atomic<float>* pPadGate = nullptr;   std::atomic<float>* pGlide = nullptr;
    std::atomic<float>* pVibRate = nullptr;   std::atomic<float>* pVibDepth = nullptr;
    std::atomic<float>* pAttack = nullptr;    std::atomic<float>* pRelease = nullptr;
    std::atomic<float>* pFormant = nullptr;   std::atomic<float>* pThroat = nullptr;
    std::atomic<float>* pBreath = nullptr;    std::atomic<float>* pGrowl = nullptr;
    std::atomic<float>* pJitter = nullptr;    std::atomic<float>* pRobot = nullptr;
    std::atomic<float>* pCrush = nullptr;     std::atomic<float>* pDelayTime = nullptr;
    std::atomic<float>* pDelayFb = nullptr;   std::atomic<float>* pDelayMix = nullptr;
    std::atomic<float>* pDelayTone = nullptr; std::atomic<float>* pRoom = nullptr;
    std::atomic<float>* pGain = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonkMachineProcessor)
};
