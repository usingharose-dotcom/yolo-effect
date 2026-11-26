#include "PluginEditor.h"

using juce::Colour;

//==============================================================================
const Theme& themeFor (int mode)
{
    static const Theme themes[3] = {
        // Monk: carved wood + brass
        { Colour (0xff3b2112), Colour (0xffb8862b), Colour (0xff25130a), Colour (0xff8a6420), Colour (0xfff0c870), Colour (0xffffd27a),
          Colour (0xfff0c870), Colour (0xffc9a46a), Colour (0xff1c0d06), Colour (0xffd9a440),
          Colour (0xffd9a845), Colour (0xff8a6420), Colour (0xff2a1408), Colour (0xff2a1408),
          Colour (0xfff3e6c8), Colour (0xff1a0d06), Colour (0xffc7a66a), Colour (0xffd9a440) },
        // Zombie: grimy moss + bone
        { Colour (0xff262b1d), Colour (0xff3f4a2c), Colour (0xff161b10), Colour (0xff333d24), Colour (0xff9be05a), Colour (0xffa6f06a),
          Colour (0xff9be05a), Colour (0xff8f9c78), Colour (0xff0e140b), Colour (0xff7cd23a),
          Colour (0xff5b6740), Colour (0xff2c341f), Colour (0xffd6e8b8), Colour (0xffa6f06a),
          Colour (0xffc9c2a4), Colour (0xff161a10), Colour (0xff7a7458), Colour (0xff7cf03a) },
        // Animatronic: riveted steel + red
        { Colour (0xff4a4e55), Colour (0xff1e2024), Colour (0xff25282d), Colour (0xff121417), Colour (0xffe8e8e8), Colour (0xffff4a3a),
          Colour (0xfff2f2f2), Colour (0xffff8a7a), Colour (0xff0b0e0c), Colour (0xffaabeb4),
          Colour (0xff9aa0a8), Colour (0xff565b63), Colour (0xff111111), Colour (0xffff4a3a),
          Colour (0xffe4e6e9), Colour (0xff111111), Colour (0xff9aa0a8), Colour (0xffff3b2f) },
    };
    return themes[juce::jlimit (0, 2, mode)];
}

juce::Font themeFont (int mode, float height)
{
    switch (mode)
    {
        case 1:  return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), height, juce::Font::bold | juce::Font::italic));
        case 2:  return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::bold));
        default: return juce::Font (juce::FontOptions (juce::Font::getDefaultSerifFontName(), height, juce::Font::bold));
    }
}

//==============================================================================
void ThemeLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                 float startAngle, float endAngle, juce::Slider&)
{
    const float r  = (float) juce::jmin (w, h) * 0.5f - 3.f;
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto knob = juce::Rectangle<float> (r * 2.f, r * 2.f).withCentre ({ cx, cy });
    const auto& th = themeFor (mode);

    // scale ticks
    g.setColour (th.dim.withAlpha (0.5f));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.f;
        const juce::Point<float> p1 (cx + std::sin (a) * (r + 1.5f), cy - std::cos (a) * (r + 1.5f));
        const juce::Point<float> p2 (cx + std::sin (a) * (r + 3.f),  cy - std::cos (a) * (r + 3.f));
        g.drawLine ({ p1, p2 }, 0.8f);
    }

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (knob.translated (0.f, 2.f));

    juce::Colour pointer;
    if (mode == 2)
    {
        juce::ColourGradient cg (Colour (0xfff4f4f4), cx - r * 0.3f, cy - r * 0.45f, Colour (0xff6e6e6e), cx + r * 0.6f, cy + r * 0.9f, true);
        cg.addColour (0.5, Colour (0xffbdbdbd));
        g.setGradientFill (cg);
        g.fillEllipse (knob);
        g.setColour (Colour (0xff3a3a3a));                              // knurled rim
        for (int i = 0; i < 36; ++i)
        {
            const float an = juce::MathConstants<float>::twoPi * (float) i / 36.f;
            g.drawLine (cx + std::sin (an) * r * 0.82f, cy - std::cos (an) * r * 0.82f,
                        cx + std::sin (an) * r,         cy - std::cos (an) * r, 0.8f);
        }
        g.setColour (Colour (0xffd8d8d8)); g.drawEllipse (knob.reduced (r * 0.2f), 0.8f);
        g.setColour (Colour (0xff222222)); g.drawEllipse (knob, 1.5f);
        pointer = Colour (0xffff3b2f);
    }
    else
    {
        const Colour light = mode == 0 ? Colour (0xfffff0b8) : Colour (0xfff1ead0);
        const Colour mid   = mode == 0 ? Colour (0xffd9a440) : Colour (0xffb0a682);
        const Colour dark  = mode == 0 ? Colour (0xff7a520f) : Colour (0xff4d4734);
        juce::ColourGradient grad (light, cx - r * 0.35f, cy - r * 0.45f, dark, cx + r * 0.7f, cy + r * 0.9f, true);
        grad.addColour (0.45, mid);
        g.setGradientFill (grad);
        g.fillEllipse (knob);
        g.setColour (mode == 0 ? Colour (0xff3a2406) : Colour (0xff1a1a10)); g.drawEllipse (knob, 1.f);
        pointer = mode == 0 ? Colour (0xff2a1405) : Colour (0xff7cf03a);
    }

    const juce::Point<float> a (cx + std::sin (angle) * r * 0.3f,  cy - std::cos (angle) * r * 0.3f);
    const juce::Point<float> b (cx + std::sin (angle) * r * 0.85f, cy - std::cos (angle) * r * 0.85f);
    if (mode != 0) { g.setColour (pointer.withAlpha (0.3f)); g.drawLine ({ a, b }, 5.f); }
    g.setColour (pointer);
    g.drawLine ({ a, b }, 2.2f);
}

void ThemeLnF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                     bool highlighted, bool down)
{
    const auto& th = themeFor (mode);
    auto r = b.getLocalBounds().toFloat().reduced (0.5f, 1.f);
    const bool on = b.getToggleState() || down;
    Colour top = on ? th.btnBot.darker (0.3f) : th.btnTop;
    Colour bot = on ? th.btnTop.darker (0.2f) : th.btnBot;
    if (highlighted && ! on) { top = top.brighter (0.15f); bot = bot.brighter (0.1f); }

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (0.f, on ? 0.5f : 1.5f), 4.f);
    if (on) r = r.translated (0.f, 1.f);
    g.setGradientFill (juce::ColourGradient (top, 0.f, r.getY(), bot, 0.f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.drawRoundedRectangle (r, 4.f, 1.f);
    if (! on) { g.setColour (juce::Colours::white.withAlpha (0.2f)); g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + 3.f, r.getRight() - 3.f); }
}

juce::Font ThemeLnF::getTextButtonFont (juce::TextButton&, int h)     { return themeFont (mode, juce::jmin (12.f, (float) h * 0.6f)); }
juce::Font ThemeLnF::getLabelFont (juce::Label& l)                    { return themeFont (mode, l.getFont().getHeight()); }
juce::Font ThemeLnF::getSliderPopupFont (juce::Slider&)               { return themeFont (mode, 11.f); }

//==============================================================================
ChantPad::ChantPad (MonkMachineProcessor& p)
    : proc (p),
      vowel (p.apvts.getParameter ("vowel")),
      pitch (p.apvts.getParameter ("padPitch")),
      gate  (p.apvts.getParameter ("padGate")),
      rng (1234)
{
    for (int i = 0; i < 12; ++i)
        fog.push_back ({ rng.nextFloat() * 214.f, rng.nextFloat() * 226.f, 30.f + rng.nextFloat() * 45.f, 0.1f + rng.nextFloat() * 0.3f });
    setMouseCursor (juce::MouseCursor::CrosshairCursor);
    startTimerHz (30);
}

ChantPad::~ChantPad() { stopTimer(); releaseGate(); }

void ChantPad::timerCallback()
{
    ++frame;
    for (auto& f : fog) { f.x += f.s; if (f.x - f.r > (float) getWidth()) f.x = -f.r; }
    repaint();
}

void ChantPad::updateFromMouse (juce::Point<float> pos)
{
    vowel->setValueNotifyingHost (juce::jlimit (0.f, 1.f, pos.x / (float) getWidth()));
    pitch->setValueNotifyingHost (juce::jlimit (0.f, 1.f, 1.f - pos.y / (float) getHeight()));
}

void ChantPad::mouseDown (const juce::MouseEvent& e)
{
    dragging = true;
    vowel->beginChangeGesture(); pitch->beginChangeGesture(); gate->beginChangeGesture();
    updateFromMouse (e.position);
    gate->setValueNotifyingHost (1.f);
}

void ChantPad::mouseDrag (const juce::MouseEvent& e) { if (dragging) updateFromMouse (e.position); }
void ChantPad::mouseUp (const juce::MouseEvent&)     { releaseGate(); }

void ChantPad::releaseGate()
{
    if (! dragging) return;
    dragging = false;
    gate->setValueNotifyingHost (0.f);
    gate->endChangeGesture(); vowel->endChangeGesture(); pitch->endChangeGesture();
}

void ChantPad::drawMouth (juce::Graphics& g, int mode, juce::Point<float> c, float mw, float mh, float env)
{
    if (mode == 2)   // speaker grille
    {
        auto r = juce::Rectangle<float> (juce::jmax (3.f, mw), juce::jmax (3.f, mh)).withCentre (c);
        g.setColour (Colour (0xffff5a46).withAlpha (0.5f + 0.5f * env));
        g.drawRect (r, 2.5f);
        g.setColour (Colour (0xdd080604));
        g.fillRect (r.reduced (2.5f));
        g.setColour (Colour (0xffff8c78).withAlpha (0.7f));
        for (int i = 1; i < 4; ++i) g.fillRect (r.getX() + r.getWidth() * (float) i / 4.f - 1.f, r.getY(), 2.f, juce::jmax (2.f, r.getHeight() * 0.3f));
        return;
    }
    if (mode == 1)   // ragged, drooling mouth
    {
        g.setColour (Colour (0xff7cf03a).withAlpha (0.08f + 0.2f * env));
        g.fillEllipse (juce::Rectangle<float> (mw * 2.4f, mh * 2.4f).withCentre (c));
        juce::Path p;
        for (int i = 0; i <= 16; ++i)
        {
            const float an = (float) i / 16.f * juce::MathConstants<float>::twoPi;
            const float rr = 1.f + std::sin ((float) i * 2.7f + (float) frame * 0.3f) * 0.15f * env;
            const juce::Point<float> pt (c.x + std::cos (an) * mw * 0.5f * rr, c.y + std::sin (an) * mh * 0.5f * rr);
            if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
        }
        p.closeSubPath();
        g.setColour (Colour (0xff050803)); g.fillPath (p);
        g.setColour (Colour (0xffa0f064).withAlpha (0.5f + 0.5f * env)); g.strokePath (p, juce::PathStrokeType (2.f));
        return;
    }
    g.setColour (Colour (0xffffd27a).withAlpha (0.12f + 0.25f * env));
    g.fillEllipse (juce::Rectangle<float> (mw * 2.4f, mh * 2.4f).withCentre (c));
    g.setColour (Colour (0xffffd27a).withAlpha (0.5f + 0.5f * env));
    g.drawEllipse (juce::Rectangle<float> (juce::jmax (3.f, mw), juce::jmax (3.f, mh)).withCentre (c), 2.5f);
    g.setColour (Colour (0xe6080402));
    g.fillEllipse (juce::Rectangle<float> (juce::jmax (1.f, mw - 5.f), juce::jmax (1.f, mh - 5.f)).withCentre (c));
}

void ChantPad::paint (juce::Graphics& g)
{
    const int mode = proc.getMode();
    const auto& th = themeFor (mode);
    const float W = (float) getWidth(), H = (float) getHeight();
    const float env = proc.displayEnv.load();

    g.setColour (th.padBg);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.f);

    // faint brand watermark
    g.setColour (th.text.withAlpha (0.06f));
    g.setFont (themeFont (mode, 50.f));
    g.drawText ("YOLO", getLocalBounds(), juce::Justification::centred);

    if (mode == 0)          // incense smoke
    {
        for (int i = 0; i < 3; ++i)
        {
            juce::Path s;
            const float bx = W * (0.15f + 0.33f * (float) i);
            for (float yy = H; yy > 0.f; yy -= 5.f)
            {
                const float xx = bx + std::sin (yy * 0.035f + (float) frame * 0.02f + (float) i) * 10.f * (1.f - yy / H)
                                    + std::sin (yy * 0.013f + (float) frame * 0.01f) * 6.f;
                if (yy >= H) s.startNewSubPath (xx, yy); else s.lineTo (xx, yy);
            }
            g.setColour (Colour (0xffe6dcc8).withAlpha (0.05f + 0.02f * (float) i));
            g.strokePath (s, juce::PathStrokeType (5.f - (float) i));
        }
        g.setColour (th.grid.withAlpha (0.6f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (4.f), 4.f, 1.5f);
    }
    else if (mode == 1)     // graveyard
    {
        g.setColour (Colour (0xff1a2314));
        const float stones[3][2] = { { 0.1f, 32.f }, { 0.44f, 44.f }, { 0.76f, 28.f } };
        for (auto& s : stones)
        {
            const float x = W * s[0], top = H - 26.f - s[1];
            juce::Path p;
            p.addRoundedRectangle (x, top, 28.f, s[1] + 4.f, 13.f, 13.f, true, true, false, false);
            g.fillPath (p);
        }
        g.setColour (Colour (0xff141b10));
        g.fillRect (0.f, H - 26.f, W, 26.f);
        for (auto& f : fog)
        {
            juce::ColourGradient fg (Colour (0x12a0be78), f.x, f.y, Colour (0x00a0be78), f.x + f.r, f.y, true);
            g.setGradientFill (fg);
            g.fillEllipse (f.x - f.r, f.y - f.r, f.r * 2.f, f.r * 2.f);
        }
    }

    // semitone grid (2 octaves)
    for (int s = 1; s < 24; ++s)
    {
        g.setColour (th.grid.withAlpha (s % 12 == 0 ? 0.35f : 0.08f));
        g.drawHorizontalLine ((int) (H * (1.f - (float) s / 24.f)), 6.f, W - 6.f);
    }
    const char* letters[] = { "U", "O", "A", "E", "I" };
    g.setFont (themeFont (mode, 14.f));
    for (int i = 0; i < 5; ++i)
    {
        const float x = 10.f + (W - 20.f) * (float) i / 4.f;
        g.setColour (th.grid.withAlpha (0.1f));
        g.drawVerticalLine ((int) x, 6.f, H - 24.f);
        g.setColour (th.text.withAlpha (0.75f));
        g.drawText (letters[i], juce::Rectangle<float> (x - 12.f, H - 22.f, 24.f, 18.f), juce::Justification::centred);
    }

    // mouth
    static const float mouthW[5] = { 13.f, 19.f, 24.f, 34.f, 37.f };
    static const float mouthH[5] = { 14.f, 22.f, 32.f, 18.f, 9.f };
    const float v = proc.displayVowel.load();
    const float pos = v * 4.f;
    const int a = juce::jmin (3, (int) pos);
    const float t = pos - (float) a;
    const float shake = (rng.nextFloat() - 0.5f) * 5.f * env * (proc.apvts.getRawParameterValue ("growl")->load()
                                                               + 0.5f * proc.apvts.getRawParameterValue ("crush")->load());
    const float mw = (mouthW[a] + (mouthW[a + 1] - mouthW[a]) * t) * (0.8f + 0.5f * env) + shake;
    const float mh = (mouthH[a] + (mouthH[a + 1] - mouthH[a]) * t) * (0.6f + 0.7f * env) + shake;
    drawMouth (g, mode, { 10.f + (W - 20.f) * v, H * (1.f - proc.displayPitch.load()) }, mw, mh, env);

    if (mode == 2)          // security-camera screen
    {
        for (int i = 0; i < 350; ++i)
        {
            g.setColour (juce::Colours::white.withAlpha (rng.nextFloat() * 0.25f));
            g.fillRect (rng.nextFloat() * W, rng.nextFloat() * H, 1.5f, 1.5f);
        }
        g.setColour (juce::Colours::black.withAlpha (0.28f));
        for (int yy = frame % 3; yy < (int) H; yy += 3) g.drawHorizontalLine (yy, 0.f, W);
        juce::ColourGradient vg (juce::Colours::transparentBlack, W * 0.5f, H * 0.5f, juce::Colours::black.withAlpha (0.7f), 0.f, 0.f, true);
        vg.addColour (0.5, juce::Colours::transparentBlack);
        g.setGradientFill (vg);
        g.fillRect (getLocalBounds());

        g.setFont (themeFont (2, 11.f));
        g.setColour (Colour (0xffe8ece9));
        g.drawText ("YOLO CAM 03", 8, 5, 120, 14, juce::Justification::centredLeft);
        g.drawText (juce::Time::getCurrentTime().formatted ("%I:%M %p"), (int) W - 88, 5, 80, 14, juce::Justification::centredRight);
        if ((frame / 15) % 2)
        {
            g.setColour (Colour (0xffff3b2f));
            g.fillEllipse (9.f, 22.f, 7.f, 7.f);
            g.drawText ("REC", 20, 19, 40, 13, juce::Justification::centredLeft);
        }
    }
    else
    {
        g.setColour (th.text.withAlpha (0.5f));
        g.setFont (themeFont (mode, 10.f));
        g.drawText (juce::String ("vowel ") + juce::String::fromUTF8 ("\xe2\x86\x92") + "   pitch " + juce::String::fromUTF8 ("\xe2\x86\x91"),
                    8, 5, 160, 13, juce::Justification::centredLeft);
    }

    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 6.f, 1.5f);
}

//==============================================================================
Face::Face (MonkMachineProcessor& p, ThemeLnF& l)
    : proc (p), lnf (l), pad (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible (pad);

    const char* names[3] = { "Monk", "Zombie", "Animatronic" };
    for (int i = 0; i < 3; ++i)
    {
        modeButtons[i].setButtonText (names[i]);
        modeButtons[i].setClickingTogglesState (false);
        modeButtons[i].onClick = [this, i] { proc.applyMode (i); applyTheme(); };
        addAndMakeVisible (modeButtons[i]);
    }
    autoButton.onClick = [this] { proc.triggerAutoVoice(); };
    addAndMakeVisible (autoButton);

    addKnob ("glide",     "Glide",     true);
    addKnob ("vibRate",   "Vib Rate",  true);
    addKnob ("vibDepth",  "Vib Depth", true);
    addKnob ("formant",   "Head",      true);
    addKnob ("throat",    "Throat",    true);
    addKnob ("breath",    "Breath",    true);
    addKnob ("attack",    "Attack",    true);
    addKnob ("release",   "Release",   true);
    addKnob ("growl",     "Growl",     true);
    addKnob ("jitter",    "Jitter",    true);
    addKnob ("robot",     "Robot",     true);
    addKnob ("crush",     "Crush",     true);
    addKnob ("delayTime", "Delay",     false);
    addKnob ("delayFb",   "Feedback",  false);
    addKnob ("delayTone", "Tone",      false);
    addKnob ("delayMix",  "Mix",       false);
    addKnob ("room",      "Room",      false);
    addKnob ("gain",      "Volume",    false);

    keyboard.setAvailableRange (36, 84);
    keyboard.setLowestVisibleKey (36);
    keyboard.setScrollButtonsVisible (false);
    addAndMakeVisible (keyboard);

    applyTheme();
    startTimerHz (15);
}

void Face::addKnob (const juce::String& id, const juce::String& name, bool voiceSection)
{
    auto k = std::make_unique<Knob>();
    k->slider.setPopupDisplayEnabled (true, true, this);
    k->slider.setTooltip (name);
    k->label.setText (name, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setFont (juce::Font (juce::FontOptions (10.f)));
    k->label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (k->slider);
    addAndMakeVisible (k->label);
    k->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->slider);
    (voiceSection ? voiceKnobs : spaceKnobs).push_back (std::move (k));
}

void Face::applyTheme()
{
    const int m = proc.getMode();
    shownMode = m;
    lnf.mode = m;
    const auto& th = themeFor (m);

    for (int i = 0; i < 3; ++i)
    {
        modeButtons[i].setToggleState (i == m, juce::dontSendNotification);
        modeButtons[i].setColour (juce::TextButton::textColourOffId, th.btnText);
        modeButtons[i].setColour (juce::TextButton::textColourOnId,  th.btnOnText);
    }
    autoButton.setColour (juce::TextButton::textColourOffId, th.btnText);
    autoButton.setColour (juce::TextButton::textColourOnId,  th.btnOnText);

    for (auto* list : { &voiceKnobs, &spaceKnobs })
        for (auto& k : *list)
            k->label.setColour (juce::Label::textColourId, th.accent);

    lnf.setColour (juce::BubbleComponent::backgroundColourId, th.panel.brighter (0.2f));
    lnf.setColour (juce::BubbleComponent::outlineColourId,    th.panelBorder);
    lnf.setColour (juce::TooltipWindow::textColourId,         th.text);

    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,           th.whiteKey);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId,           th.blackKey);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,    th.keyLine);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,      th.keyDown.withAlpha (0.85f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, th.keyDown.withAlpha (0.25f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId,              juce::Colours::black.withAlpha (0.5f));
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId,           th.blackKey.withAlpha (0.6f));

    sendLookAndFeelChange();
    repaint();
}

void Face::timerCallback()
{
    if (proc.getMode() != shownMode) applyTheme();
    if (shownMode == 2) repaint (ledArea.expanded (6.f).getSmallestIntegerContainer());
}

void Face::resized()
{
    plateArea = { 10.f, 8.f, 50.f, 22.f };
    titleArea = { 66.f, 6.f, 168.f, 26.f };
    int x = 236;
    const int widths[3] = { 52, 58, 86 };
    for (int i = 0; i < 3; ++i) { modeButtons[i].setBounds (x, 9, widths[i], 20); x += widths[i] + 4; }
    autoButton.setBounds (kFaceW - 10 - 78, 9, 78, 20);
    ledArea = { (float) kFaceW - 14.f, 2.f, 6.f, 6.f };

    pad.setBounds (10, 40, 214, 226);
    panelArea = { 232.f, 40.f, 318.f, 226.f };

    const int cellW = 53, x0 = 232;
    auto place = [&] (std::vector<std::unique_ptr<Knob>>& list, int firstRowY)
    {
        for (size_t i = 0; i < list.size(); ++i)
        {
            const int row = (int) i / 6, col = (int) i % 6;
            const int cx = x0 + col * cellW, cy = firstRowY + row * 64;
            list[i]->label.setBounds (cx, cy, cellW, 12);
            list[i]->slider.setBounds (cx + 4, cy + 12, cellW - 8, 46);
        }
    };
    place (voiceKnobs, 56);
    place (spaceKnobs, 204);

    keyboard.setBounds (10, 274, kFaceW - 20, 44);
    keyboard.setKeyWidth ((float) (kFaceW - 20) / 29.f);   // 29 white keys in C2..C6
}

void Face::paintBackground (juce::Graphics& g, const Theme& th)
{
    const auto b = getLocalBounds().toFloat();
    const float W = b.getWidth(), H = b.getHeight();
    juce::Random r (42);

    if (shownMode == 0)
    {
        g.fillAll (th.bg);
        for (int i = 0; i < 160; ++i)                                  // wood grain
        {
            const float x = r.nextFloat() * W;
            g.setColour ((r.nextBool() ? Colour (0xff5a3520) : Colour (0xff24120a)).withAlpha (0.15f + r.nextFloat() * 0.25f));
            g.drawLine (x, 0.f, x + r.nextFloat() * 8.f - 4.f, H, 0.6f + r.nextFloat() * 1.8f);
        }
    }
    else if (shownMode == 1)
    {
        g.fillAll (th.bg);
        for (int i = 0; i < 120; ++i)                                  // grime streaks
        {
            const float y = r.nextFloat() * H;
            g.setColour (Colour (0xff1a1f13).withAlpha (0.2f + r.nextFloat() * 0.3f));
            g.drawLine (0.f, y, W, y + r.nextFloat() * 4.f - 2.f, 0.5f + r.nextFloat() * 1.5f);
        }
        const float moss[3][3] = { { 0.18f, 0.12f, 120.f }, { 0.85f, 0.85f, 160.f }, { 0.5f, 1.0f, 120.f } };
        for (auto& m : moss)
        {
            juce::ColourGradient mg (Colour (0x8c5a7832), W * m[0], H * m[1], Colour (0x005a7832), W * m[0] + m[2], H * m[1], true);
            g.setGradientFill (mg);
            g.fillEllipse (W * m[0] - m[2], H * m[1] - m[2], m[2] * 2.f, m[2] * 2.f);
        }
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (Colour (0xff62666d), 0.f, 0.f, Colour (0xff2e3137), 0.f, H, false));
        g.fillAll();
        for (int i = 0; i < 90; ++i)                                   // brushed steel
        {
            const float y = r.nextFloat() * H;
            g.setColour (juce::Colours::white.withAlpha (0.02f + r.nextFloat() * 0.04f));
            g.drawHorizontalLine ((int) y, 0.f, W);
        }
    }

    // vignette
    juce::ColourGradient vig (juce::Colours::transparentBlack, W * 0.5f, H * 0.5f, juce::Colours::black.withAlpha (shownMode == 2 ? 0.35f : 0.6f), 0.f, 0.f, true);
    vig.addColour (0.55, juce::Colours::transparentBlack);
    g.setGradientFill (vig);
    g.fillAll();

    // frame
    g.setColour (th.border);
    g.drawRoundedRectangle (b.reduced (2.f), 9.f, 4.f);
    if (shownMode == 0) { g.setColour (Colour (0xff5a3c12)); g.drawRoundedRectangle (b.reduced (4.5f), 7.f, 1.f); }
    if (shownMode == 2) { g.setColour (Colour (0xff8a8f96)); g.drawHorizontalLine (5, 10.f, W - 10.f); }

    // decorations
    auto stud = [&] (float x, float y, Colour light, Colour mid, Colour dark, float d)
    {
        juce::ColourGradient sg (light, x - d * 0.2f, y - d * 0.25f, dark, x + d * 0.5f, y + d * 0.5f, true);
        sg.addColour (0.5, mid);
        g.setGradientFill (sg);
        g.fillEllipse (x - d * 0.5f, y - d * 0.5f, d, d);
    };
    if (shownMode == 0)
    {
        for (auto pt : { juce::Point<float> (8.f, 8.f), { W - 8.f, 8.f }, { 8.f, H - 8.f }, { W - 8.f, H - 8.f } })
            stud (pt.x, pt.y, Colour (0xfffff0b8), Colour (0xffb8862b), Colour (0xff5a3c12), 7.f);
    }
    else if (shownMode == 1)
    {
        const float drips[7][2] = { { 0.03f, 9.f }, { 0.36f, 6.f }, { 0.41f, 10.f }, { 0.55f, 7.f }, { 0.62f, 11.f }, { 0.97f, 8.f }, { 0.5f, 5.f } };
        g.setColour (Colour (0xbf7cd23a));
        for (auto& d : drips)
        {
            const float x = W * d[0];
            g.fillRoundedRectangle (x, 0.f, 4.f, d[1], 2.f);
            g.fillEllipse (x - 1.f, d[1] - 3.f, 6.f, 6.f);
        }
    }
    else
    {
        g.saveState();
        g.reduceClipRegion (juce::Rectangle<int> (4, (int) H - 9, (int) W - 8, 5));
        for (float x = -10.f; x < W + 10.f; x += 14.f)
        {
            juce::Path s;
            s.addQuadrilateral (x, H, x + 7.f, H, x + 14.f, H - 10.f, x + 7.f, H - 10.f);
            g.setColour (Colour (0xffe8b923)); g.fillPath (s);
        }
        g.restoreState();
        for (auto pt : { juce::Point<float> (8.f, 8.f), { 8.f, H - 14.f }, { W - 8.f, H - 14.f } })
            stud (pt.x, pt.y, Colour (0xffeeeeee), Colour (0xff777777), Colour (0xff222222), 6.f);
    }
}

void Face::paintPlate (juce::Graphics& g, const Theme&, juce::Rectangle<float> r)
{
    if (shownMode == 0)
    {
        juce::ColourGradient pg (Colour (0xffffe7a3), 0.f, r.getY(), Colour (0xff8a6420), 0.f, r.getBottom(), false);
        pg.addColour (0.55, Colour (0xffc8952f));
        g.setGradientFill (pg); g.fillRoundedRectangle (r, 3.f);
        g.setColour (Colour (0xff4a3008)); g.drawRoundedRectangle (r, 3.f, 1.f);
        g.setColour (Colour (0x99fff0be)); g.setFont (themeFont (0, 14.f)); g.drawText ("YOLO", r.translated (0.f, 1.f), juce::Justification::centred);
        g.setColour (Colour (0xff3a1d08)); g.drawText ("YOLO", r, juce::Justification::centred);
    }
    else if (shownMode == 1)
    {
        g.saveState();
        g.addTransform (juce::AffineTransform::rotation (-0.05f, r.getCentreX(), r.getCentreY()));
        g.setColour (Colour (0xff1a2012)); g.fillRoundedRectangle (r, 3.f);
        g.setColour (Colour (0xff4b5a30)); g.drawRoundedRectangle (r, 3.f, 2.f);
        g.setFont (themeFont (1, 15.f));
        g.setColour (Colour (0x557cf03a)); g.drawText ("YOLO", r.expanded (1.f), juce::Justification::centred);
        g.setColour (Colour (0xffa6f06a)); g.drawText ("YOLO", r, juce::Justification::centred);
        g.restoreState();
    }
    else
    {
        g.saveState();
        g.reduceClipRegion (r.getSmallestIntegerContainer());
        g.setColour (Colour (0xffe8b923)); g.fillRect (r);
        g.setColour (Colour (0xffd4a514));
        for (float x = r.getX() - 20.f; x < r.getRight(); x += 10.f)
        {
            juce::Path s; s.addQuadrilateral (x, r.getBottom(), x + 5.f, r.getBottom(), x + 5.f + r.getHeight(), r.getY(), x + r.getHeight(), r.getY());
            g.fillPath (s);
        }
        g.restoreState();
        g.setColour (Colour (0xff111111)); g.drawRect (r, 1.5f);
        g.setFont (themeFont (2, 14.f)); g.drawText ("YOLO", r, juce::Justification::centred);
    }
}

void Face::paint (juce::Graphics& g)
{
    const auto& th = themeFor (shownMode);
    paintBackground (g, th);
    paintPlate (g, th, plateArea);

    static const char* titles[3] = { "Monk machine", "Zombie machine", "Animatronic" };
    g.setFont (themeFont (shownMode, shownMode == 1 ? 18.f : 16.f));
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawText (titles[shownMode], titleArea.translated (0.f, 1.5f), juce::Justification::centredLeft);
    g.setColour (th.hi);
    g.drawText (titles[shownMode], titleArea, juce::Justification::centredLeft);

    // knob panel (recessed)
    g.setColour (th.panel);
    g.fillRoundedRectangle (panelArea, 6.f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.6f), 0.f, panelArea.getY(),
                                             juce::Colours::transparentBlack, 0.f, panelArea.getY() + 10.f, false));
    g.fillRoundedRectangle (panelArea, 6.f);
    g.setColour (th.panelBorder);
    g.drawRoundedRectangle (panelArea, 6.f, 1.5f);

    g.setColour (th.accent.withAlpha (0.85f));
    g.setFont (themeFont (shownMode, 9.f));
    g.drawText ("VOICE", (int) panelArea.getX() + 8, 43, 80, 11, juce::Justification::centredLeft);
    g.drawText ("SPACE", (int) panelArea.getX() + 8, 191, 80, 11, juce::Justification::centredLeft);
    g.setColour (th.panelBorder.withAlpha (0.8f));
    g.drawHorizontalLine (188, panelArea.getX() + 8.f, panelArea.getRight() - 8.f);

    // keyboard bezel
    g.setColour (th.border);
    g.drawRoundedRectangle (keyboard.getBounds().toFloat().expanded (1.5f), 3.f, 2.f);

    if (shownMode == 2)     // warning LED
    {
        const float env = proc.displayEnv.load();
        g.setColour (Colour (0xffff3b2f).withAlpha (0.25f * env));
        g.fillEllipse (ledArea.expanded (4.f));
        g.setColour (Colour (0xffff3b2f).withAlpha (0.4f + 0.6f * env));
        g.fillEllipse (ledArea);
    }
}

//==============================================================================
MonkMachineEditor::MonkMachineEditor (MonkMachineProcessor& p)
    : AudioProcessorEditor (&p), face (p, lnf)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (face);
    face.setBounds (0, 0, kFaceW, kFaceH);

    setResizable (true, true);
    setResizeLimits ((int) (kFaceW * 0.8f), (int) (kFaceH * 0.8f), kFaceW * 2, kFaceH * 2);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) kFaceW / (double) kFaceH);
    setSize (kFaceW, kFaceH);
}

MonkMachineEditor::~MonkMachineEditor()
{
    setLookAndFeel (nullptr);
}

void MonkMachineEditor::resized()
{
    face.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) kFaceW));
}
