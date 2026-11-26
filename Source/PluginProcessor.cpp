#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::NormalisableRange<float> skewed (float lo, float hi, float centre)
    {
        juce::NormalisableRange<float> r (lo, hi);
        r.setSkewForCentre (centre);
        return r;
    }

    // Knob presets per mode (parameter id -> value), matching the preview
    struct PresetValue { const char* id; float v[3]; };   // monk, zombie, animatronic
    const PresetValue kPresets[] = {
        { "glide",     { 120.f, 320.f,  40.f } },
        { "vibRate",   {   5.f,   3.f,   7.f } },
        { "vibDepth",  {  30.f,   0.f,   8.f } },
        { "formant",   {  1.0f, 0.82f,  0.9f } },
        { "throat",    {  0.1f, 0.55f,  0.3f } },
        { "breath",    { 0.12f, 0.45f, 0.05f } },
        { "attack",    {  80.f, 140.f,  20.f } },
        { "release",   { 500.f, 350.f, 250.f } },
        { "growl",     {  0.0f, 0.75f, 0.15f } },
        { "jitter",    { 0.05f,  0.6f,  0.0f } },
        { "robot",     {  0.0f,  0.0f, 0.65f } },
        { "crush",     {  0.0f,  0.0f, 0.45f } },
        { "delayTime", { 375.f,  90.f, 240.f } },
        { "delayFb",   {  0.4f, 0.15f, 0.35f } },
        { "delayTone", { 3500.f, 1500.f, 2500.f } },
        { "delayMix",  { 0.22f,  0.1f, 0.25f } },
        { "room",      {  0.4f,  0.3f,  0.3f } },
    };
}

juce::AudioProcessorValueTreeState::ParameterLayout MonkMachineProcessor::createLayout()
{
    using F = juce::AudioParameterFloat;
    using A = juce::AudioParameterFloatAttributes;
    using R = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "mode", 1 }, "Mode",
                                                               juce::StringArray { "Monk", "Zombie", "Animatronic" }, 0));
    p.push_back (std::make_unique<F> (juce::ParameterID { "vowel", 1 },     "Vowel",      R (0.f, 1.f), 0.3f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "padPitch", 1 },  "Pad Pitch",  R (0.f, 1.f), 0.45f));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "padGate", 1 }, "Pad Sing", false));

    p.push_back (std::make_unique<F> (juce::ParameterID { "glide", 1 },     "Glide",      skewed (0.f, 1000.f, 150.f),  120.f,  A().withLabel ("ms")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "vibRate", 1 },   "Vib Rate",   skewed (0.5f, 10.f, 5.f),     5.f,    A().withLabel ("Hz")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "vibDepth", 1 },  "Vib Depth",  R (0.f, 100.f),               30.f,   A().withLabel ("ct")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "formant", 1 },   "Head Size",  skewed (0.75f, 1.33f, 1.f),   1.f,    A().withLabel ("x")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "throat", 1 },    "Throat",     R (0.f, 1.f),                 0.1f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "breath", 1 },    "Breath",     R (0.f, 1.f),                 0.12f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "attack", 1 },    "Attack",     skewed (1.f, 2000.f, 120.f),  80.f,   A().withLabel ("ms")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "release", 1 },   "Release",    skewed (10.f, 4000.f, 500.f), 500.f,  A().withLabel ("ms")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "growl", 1 },     "Growl",      R (0.f, 1.f),                 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "jitter", 1 },    "Jitter",     R (0.f, 1.f),                 0.05f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "robot", 1 },     "Robot",      R (0.f, 1.f),                 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "crush", 1 },     "Crush",      R (0.f, 1.f),                 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "delayTime", 1 }, "Delay Time", skewed (20.f, 2000.f, 400.f), 375.f,  A().withLabel ("ms")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "delayFb", 1 },   "Feedback",   R (0.f, 0.95f),               0.4f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "delayTone", 1 }, "Delay Tone", skewed (500.f, 12000.f, 3000.f), 3500.f, A().withLabel ("Hz")));
    p.push_back (std::make_unique<F> (juce::ParameterID { "delayMix", 1 },  "Delay Mix",  R (0.f, 1.f),                 0.22f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "room", 1 },      "Room",       R (0.f, 1.f),                 0.4f));
    p.push_back (std::make_unique<F> (juce::ParameterID { "gain", 1 },      "Volume",     R (-36.f, 6.f),               0.f,    A().withLabel ("dB")));

    return { p.begin(), p.end() };
}

MonkMachineProcessor::MonkMachineProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id); };
    pMode = get ("mode");
    pVowel = get ("vowel");        pPadPitch = get ("padPitch");  pPadGate = get ("padGate");
    pGlide = get ("glide");        pVibRate = get ("vibRate");    pVibDepth = get ("vibDepth");
    pAttack = get ("attack");      pRelease = get ("release");    pFormant = get ("formant");
    pThroat = get ("throat");      pBreath = get ("breath");      pGrowl = get ("growl");
    pJitter = get ("jitter");      pRobot = get ("robot");        pCrush = get ("crush");
    pDelayTime = get ("delayTime"); pDelayFb = get ("delayFb");   pDelayMix = get ("delayMix");
    pDelayTone = get ("delayTone"); pRoom = get ("room");         pGain = get ("gain");
}

void MonkMachineProcessor::applyMode (int mode)
{
    mode = juce::jlimit (0, 2, mode);
    auto set = [this] (const char* id, float value)
    {
        if (auto* prm = apvts.getParameter (id))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->convertTo0to1 (value));
            prm->endChangeGesture();
        }
    };
    set ("mode", (float) mode);
    for (const auto& pv : kPresets) set (pv.id, pv.v[mode]);

    static const float startVowel[3] = { 0.3f, 0.12f, 0.45f };
    static const float startPitch[3] = { 0.45f, 0.3f, 0.35f };
    set ("vowel", startVowel[mode]);
    set ("padPitch", startPitch[mode]);
}

bool MonkMachineProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MonkMachineProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    keyboardState.reset();
}

void MonkMachineProcessor::pullParams()
{
    params.mode          = (int) pMode->load();
    params.vowel         = pVowel->load();
    params.padPitch      = pPadPitch->load();
    params.padGate       = pPadGate->load() > 0.5f;
    params.glideMs       = pGlide->load();
    params.vibRateHz     = pVibRate->load();
    params.vibDepthCents = pVibDepth->load();
    params.attackMs      = pAttack->load();
    params.releaseMs     = pRelease->load();
    params.formantShift  = pFormant->load();
    params.throat        = pThroat->load();
    params.breath        = pBreath->load();
    params.growl         = pGrowl->load();
    params.jitter        = pJitter->load();
    params.robot         = pRobot->load();
    params.crush         = pCrush->load();
    params.delayMs       = pDelayTime->load();
    params.delayFeedback = pDelayFb->load();
    params.delayMix      = pDelayMix->load();
    params.delayToneHz   = pDelayTone->load();
    params.room          = pRoom->load();
    params.gainDb        = pGain->load();
}

void MonkMachineProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())                                engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())                          engine.noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())                       engine.setPitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.f * 2.f);
    else if (m.isControllerOfType (1))               engine.setModWheel ((float) m.getControllerValue() / 127.f);
    else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
}

void MonkMachineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    pullParams();
    engine.setParams (params);
    if (autoRequest.exchange (false))
        engine.triggerAutoVoice();

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (0, numSamples, meta.samplePosition);
        if (t > pos) { engine.render (L + pos, R + pos, t - pos); pos = t; }
        handleMidi (meta.getMessage());
    }
    if (pos < numSamples)
        engine.render (L + pos, R + pos, numSamples - pos);

    displayVowel.store (engine.getCurrentVowel());
    displayPitch.store (engine.getDisplayPitch());
    displayEnv.store (engine.getEnvelope());
}

void MonkMachineProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MonkMachineProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            if (auto* gate = apvts.getParameter ("padGate"))
                gate->setValueNotifyingHost (0.f);
        }
}

juce::AudioProcessorEditor* MonkMachineProcessor::createEditor()
{
    return new MonkMachineEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonkMachineProcessor();
}
