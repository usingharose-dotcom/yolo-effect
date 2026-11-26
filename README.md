# YOLO Monk Machine

A singing-voice synth plugin (**VST3**, plus AU and Standalone on Mac) in the spirit of the classic Delay Lama.
It sings vowels ("ooo-ooh-aah-eh-eee") from an XY pad or your MIDI keyboard, and has three modes, each with
its own sound and its own skin:

| Mode | Sound | Look |
|---|---|---|
| **Monk** | Warm chanting voice in a big temple hall | Carved wood, brass knobs, incense smoke |
| **Zombie** | Low, gravelly, wobbly groans in a dark crypt | Grimy moss, bone knobs, foggy graveyard pad |
| **Animatronic** | Metallic, crunchy robot voice in a small tin room | Riveted steel, chrome knobs, security-cam pad |

The window is compact (560 × 330) and can be resized by dragging its corner.

## Playing it

- **Pad:** click and hold to sing. Left/right picks the vowel **U O A E I**; up/down sets the pitch.
- **MIDI / on-screen keys:** plays one note at a time; overlapping notes glide. Mod wheel brightens the vowel, pitch bend is ±2 semitones.
- **Mode buttons:** switch skin and sound, and load that mode's knob settings.
- **Auto voice:** performs a short phrase in the current mode's style (a chant, a groan, or a choppy robot line).
- **Knobs:** drag up and down; the value pops up while you drag.

| Knob | What it does |
|---|---|
| Glide | Slide time between notes |
| Vib Rate / Vib Depth | Vibrato speed and amount (fades in after each note) |
| Head | Head size: lower = bigger, deeper voice |
| Throat | Octave-down undertone |
| Breath | Airy noise |
| Attack / Release | Fade in / fade out time |
| Growl | Gravelly rattle and grit |
| Jitter | Unsteady, wobbly pitch |
| Robot | Metallic ring-modulated tone |
| Crush | Lo-fi digital crunch |
| Delay / Feedback / Tone / Mix | Ping-pong stereo echo |
| Room | How much of the mode's room reverb you hear |
| Volume | Output level (soft-clipped) |

All of these, plus Mode, Vowel, Pad Pitch and Pad Sing, can be automated in your DAW.

## Getting the VST3

### Option A: let GitHub build it (no coding tools needed)
1. Create a new repository on GitHub and upload everything in this folder (including the hidden `.github` folder).
2. Open the **Actions** tab. The "Build VST3" workflow runs automatically (or click **Run workflow**).
3. When it finishes (about 10 minutes), download **YOLO-Monk-Machine-VST3-Windows** or **-macOS** from the run's Artifacts.
4. Unzip it and copy `YOLO Monk Machine.vst3` into your plugin folder (below), then rescan plugins in your DAW.

### Option B: build it yourself
You need **CMake 3.22+**, **git** and a C++ compiler (Visual Studio 2022 with "Desktop development with C++" on Windows,
Xcode Command Line Tools on Mac). From this folder:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The first run downloads JUCE (the plugin framework), which takes a minute or two.
The plugin ends up in `build/YoloMonkMachine_artefacts/Release/VST3/`.

**Plugin folders**
- Windows: `C:\Program Files\Common Files\VST3`
- macOS: copied for you to `~/Library/Audio/Plug-Ins/VST3` (and `Components` for AU)
- Linux: `~/.vst3` (install `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype-dev libfontconfig1-dev` first)

On Mac, a plugin you build yourself may be blocked by Gatekeeper the first time. If your DAW won't load it, run
`xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"YOLO Monk Machine.vst3"`.

## Listening test without JUCE

The sound engine (`Source/MonkEngine.h`) is plain C++:

```bash
g++ -O2 -std=c++17 -ISource tests/render_test.cpp -o render_test && ./render_test   # writes demo.wav
```

## How it works

A band-limited glottal pulse (Rosenberg model) with slight natural pitch and loudness drift goes through 5 formant
resonators tuned to bass-singer vowels. Then come growl (amplitude flutter + drive), robot (ring modulation),
crush (amplitude quantising), a ping-pong delay and a Freeverb-style room whose character depends on the mode.

All sounds are synthesized and all artwork is drawn in code; nothing is taken from Delay Lama or any game.
