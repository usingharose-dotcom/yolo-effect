// Dev tool: renders the real plugin editor for each mode to PNG and runs audio through the processor.
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir (argc > 1 ? juce::String (argv[1]) : juce::File::getCurrentWorkingDirectory().getFullPathName());

    MonkMachineProcessor proc;
    proc.setPlayConfigDetails (0, 2, 48000.0, 512);
    proc.prepareToPlay (48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> ed;
    const char* names[3] = { "monk", "zombie", "animatronic" };
    for (int m = 0; m < 3; ++m)
    {
        proc.applyMode (m);

        // play a note for half a second so the pad shows a singing mouth
        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 45 + m * 3, (juce::uint8) 100), 0);
        float peak = 0.f; bool finite = true;
        for (int b = 0; b < 48; ++b)
        {
            proc.processBlock (buf, midi);
            midi.clear();
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 512; ++i)
                {
                    const float v = buf.getSample (ch, i);
                    finite = finite && std::isfinite (v);
                    peak = juce::jmax (peak, std::abs (v));
                }
        }
        // a fresh editor picks up the current mode's skin
        ed.reset (proc.createEditor());
        ed->setSize (kFaceW, kFaceH);

        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::File f = outDir.getChildFile (juce::String ("ui_") + names[m] + ".png");
        f.deleteFile();
        juce::FileOutputStream os (f);
        juce::PNGImageFormat().writeImageToStream (img, os);
        std::printf ("%s: peak %.3f %s -> %s\n", names[m], peak, finite ? "finite" : "NON-FINITE", f.getFullPathName().toRawUTF8());

        juce::MidiBuffer off; off.addEvent (juce::MidiMessage::allNotesOff (1), 0);
        proc.processBlock (buf, off);
    }

    // state round-trip
    juce::MemoryBlock state;
    proc.getStateInformation (state);
    MonkMachineProcessor proc2;
    proc2.setStateInformation (state.getData(), (int) state.getSize());
    std::printf ("state round-trip: mode %d (expected 2)\n", proc2.getMode());

    ed.reset();
    return 0;
}
