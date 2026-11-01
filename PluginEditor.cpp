#include "PluginEditor.h"

// ─────────────────────────────────────────────────────────────────────────────
static constexpr int kKnobSize  = 44;
static constexpr int kLabelH    = 13;
static constexpr int kKnobTotal = kKnobSize + kLabelH;
static constexpr int kKnobGap   = 6;
static constexpr int kTopPad    = 18;

// ═════════════════════════════════════════════════════════════════════════════
// YamahaLAF
// ═════════════════════════════════════════════════════════════════════════════
void YamahaLAF::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                  float pos, float startAngle, float endAngle,
                                  juce::Slider& slider)
{
    float cx  = x + w * 0.5f, cy = y + h * 0.5f;
    float R   = std::min(w, h) * 0.44f;
    float ang = startAngle + pos * (endAngle - startAngle);

    // Sombra exterior
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillEllipse(cx - R, cy - R + 2.f, R * 2.f, R * 2.f);

    // Skirt exterior
    {
        juce::ColourGradient gr(juce::Colour(0xff363636), cx - R, cy - R,
                                juce::Colour(0xff0c0c0c), cx + R, cy + R, false);
        g.setGradientFill(gr);
        g.fillEllipse(cx - R, cy - R, R * 2.f, R * 2.f);
    }

    // ── Tick marks grabados en el skirt ───────────────────────────────────────
    for (int i = 0; i <= 10; ++i)
    {
        float t  = (float)i / 10.f;
        float a  = startAngle + t * (endAngle - startAngle);
        bool major = (i == 0 || i == 5 || i == 10);
        float r1 = R * (major ? 0.84f : 0.88f);
        float r2 = R * 0.97f;
        float x1 = cx + std::sin(a) * r1, y1 = cy - std::cos(a) * r1;
        float x2 = cx + std::sin(a) * r2, y2 = cy - std::cos(a) * r2;
        g.setColour(juce::Colours::white.withAlpha(major ? 0.22f : 0.10f));
        g.drawLine(x1, y1, x2, y2, major ? 1.3f : 0.7f);
    }

    // Track gris
    float tr = R * 0.78f;
    {
        juce::Path p;
        p.addCentredArc(cx, cy, tr, tr, 0.f, startAngle, endAngle, true);
        g.setColour(GC2020Colors::arcBg);
        g.strokePath(p, juce::PathStrokeType(3.f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Arc activo dorado con gradiente
    {
        juce::Path p;
        p.addCentredArc(cx, cy, tr, tr, 0.f, startAngle, ang, true);
        juce::ColourGradient gr(GC2020Colors::yamahaGold, cx - tr, cy,
                                GC2020Colors::goldDim,    cx + tr, cy, false);
        g.setGradientFill(gr);
        g.strokePath(p, juce::PathStrokeType(3.f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Cuerpo del knob con gradiente radial premium
    float br = R * 0.60f;
    {
        juce::ColourGradient gr(juce::Colour(0xff404040), cx - br * 0.4f, cy - br * 0.4f,
                                juce::Colour(0xff0a0a0a), cx + br * 0.5f, cy + br * 0.6f, false);
        g.setGradientFill(gr);
        g.fillEllipse(cx - br, cy - br, br * 2.f, br * 2.f);
    }

    // Anillo grabado interior
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.drawEllipse(cx - br, cy - br, br * 2.f, br * 2.f, 1.f);

    // Punto indicador con halo
    float dd   = br * 0.68f;
    float dotX = cx + std::sin(ang) * dd;
    float dotY = cy - std::cos(ang) * dd;
    float dr   = br * 0.09f;
    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.fillEllipse(dotX - dr * 2.2f, dotY - dr * 2.2f, dr * 4.4f, dr * 4.4f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(dotX - dr, dotY - dr, dr * 2.f, dr * 2.f);

    // Especular superior
    juce::ColourGradient spec(juce::Colours::white.withAlpha(0.16f),
                              cx - br * 0.3f, cy - br * 0.5f,
                              juce::Colours::transparentWhite,
                              cx + br * 0.2f, cy, false);
    g.setGradientFill(spec);
    g.fillEllipse(cx - br * 0.7f, cy - br * 0.85f, br * 0.9f, br * 0.65f);

    // ── Números en skirt: 0, 5, 10 (grabados) ────────────────────────────────
    g.setFont(juce::Font("Arial", 6.5f, juce::Font::bold));
    for (int i = 0; i <= 10; i += 5)
    {
        float t  = (float)i / 10.f;
        float a  = startAngle + t * (endAngle - startAngle);
        float tx = cx + std::sin(a) * (R * 0.74f);
        float ty = cy - std::cos(a) * (R * 0.74f);
        g.setColour(juce::Colours::white.withAlpha(0.28f));
        g.drawText(juce::String(i), tx - 7.f, ty - 4.f, 14.f, 8.f,
                   juce::Justification::centred, false);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void YamahaLAF::drawButtonBackground(juce::Graphics& g, juce::Button& btn,
                                      const juce::Colour&, bool hi, bool dn)
{
    auto b  = btn.getLocalBounds().toFloat().reduced(1.f);
    bool on = btn.getToggleState();

    // Sombra
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(b.translated(0.f, 1.f), 3.f);

    // Cuerpo metálico
    juce::ColourGradient bg(
        dn ? juce::Colour(0xff0e0e0e) : juce::Colour(0xff2c2c2c), b.getX(), b.getY(),
        dn ? juce::Colour(0xff080808) : juce::Colour(0xff181818), b.getX(), b.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(b, 3.f);

    // Borde exterior
    g.setColour(hi ? juce::Colour(0xff666666) : juce::Colour(0xff3a3a3a));
    g.drawRoundedRectangle(b, 3.f, 0.8f);

    // Highlight bisel
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(b.reduced(1.f), 2.5f, 0.5f);

    // LED de estado
    float lr = 4.f;
    float lx = b.getCentreX() - lr;
    float ly = b.getY() + b.getHeight() * 0.22f - lr;

    if (on)
    {
        g.setColour(GC2020Colors::ledGreen.withAlpha(0.35f));
        g.fillEllipse(lx - 3.f, ly - 3.f, lr * 2.f + 6.f, lr * 2.f + 6.f);
    }

    juce::ColourGradient lg(
        on ? GC2020Colors::ledGreen.brighter(0.5f) : GC2020Colors::ledGreenDim,
        lx, ly,
        on ? GC2020Colors::ledGreen.darker(0.4f) : GC2020Colors::ledGreenDim.darker(0.5f),
        lx + lr * 2.f, ly + lr * 2.f, false);
    g.setGradientFill(lg);
    g.fillEllipse(lx, ly, lr * 2.f, lr * 2.f);

    if (on)
    {
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.fillEllipse(lx + lr * 0.2f, ly + lr * 0.1f, lr * 0.8f, lr * 0.5f);
    }
}

void YamahaLAF::drawButtonText(juce::Graphics& g, juce::TextButton& btn, bool, bool)
{
    g.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    g.setColour(btn.getToggleState() ? GC2020Colors::textWhite : GC2020Colors::textDim);
    auto b = btn.getLocalBounds().toFloat();
    g.drawText(btn.getButtonText(),
               b.getX(), b.getCentreY(), b.getWidth(), b.getHeight() * 0.5f,
               juce::Justification::centred, false);
}

// ═════════════════════════════════════════════════════════════════════════════
// ValueTooltip
// ═════════════════════════════════════════════════════════════════════════════
ValueTooltip::ValueTooltip()
{
    setInterceptsMouseClicks(false, false);
    setAlwaysOnTop(true);
    setVisible(false);
}

void ValueTooltip::show(const juce::String& text, juce::Point<int> pos)
{
    currentText = text;
    setSize(80, 22);
    setTopLeftPosition(pos.x - 40, pos.y - 28);
    alpha = 1.f;
    setVisible(true);
    repaint();
    startTimerHz(20);
}

void ValueTooltip::timerCallback()
{
    alpha -= 0.06f;
    if (alpha <= 0.f) { alpha = 0.f; setVisible(false); stopTimer(); }
    repaint();
}

void ValueTooltip::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a1a).withAlpha(alpha * 0.92f));
    g.fillRoundedRectangle(b, 5.f);
    g.setColour(GC2020Colors::yamahaGold.withAlpha(alpha * 0.8f));
    g.drawRoundedRectangle(b, 5.f, 0.8f);
    g.setFont(juce::Font("Arial", 9.5f, juce::Font::bold));
    g.setColour(juce::Colours::white.withAlpha(alpha));
    g.drawText(currentText, b, juce::Justification::centred, false);
}

// ═════════════════════════════════════════════════════════════════════════════
// GRMeter
// ═════════════════════════════════════════════════════════════════════════════
void GRMeter::drawLEDSegment(juce::Graphics& g, juce::Rectangle<float> b,
                              juce::Colour col, bool on)
{
    // Casing grabado
    g.setColour(juce::Colour(0xff070707));
    g.fillRoundedRectangle(b, 2.f);

    if (on)
    {
        // Glow suave multiplicado por ledPulseAlpha
        g.setColour(col.withAlpha(0.28f * ledPulseAlpha));
        g.fillRoundedRectangle(b.expanded(2.8f), 3.f);

        // Cuerpo iluminado
        juce::ColourGradient cg(col.brighter(0.4f), b.getX(), b.getY(),
                                 col.darker(0.5f),   b.getX(), b.getBottom(), false);
        g.setGradientFill(cg);
        g.fillRoundedRectangle(b, 2.f);

        // Especular
        g.setColour(juce::Colours::white.withAlpha(0.40f));
        g.fillRoundedRectangle({ b.getX() + b.getWidth() * 0.12f,
                                  b.getY() + 1.f,
                                  b.getWidth() * 0.55f,
                                  b.getHeight() * 0.38f }, 1.2f);
    }
    else
    {
        g.setColour(col.darker(3.8f).withAlpha(0.85f));
        g.fillRoundedRectangle(b, 2.f);
    }

    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.drawRoundedRectangle(b, 2.f, 0.6f);
}

void GRMeter::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced(0.8f);

    // Carcasa con efecto de vidrio
    {
        juce::ColourGradient cg(juce::Colour(0xff1e1e1e), area.getX(), area.getY(),
                                 juce::Colour(0xff090909), area.getRight(), area.getBottom(), false);
        g.setGradientFill(cg);
        g.fillRoundedRectangle(area, 4.5f);

        g.setColour(juce::Colour(0xff080808));
        g.drawRoundedRectangle(area, 4.5f, 1.3f);

        // Bisel interior
        g.setColour(juce::Colours::white.withAlpha(0.07f));
        g.drawRoundedRectangle(area.reduced(1.5f), 3.5f, 0.7f);

        // Vidrio sutil
        juce::ColourGradient glass(juce::Colours::white.withAlpha(0.04f),
                                    area.getX(), area.getY(),
                                    juce::Colours::transparentWhite,
                                    area.getX(), area.getCentreY(), false);
        g.setGradientFill(glass);
        g.fillRoundedRectangle(area.reduced(2.f), 4.f);
    }

    float pad   = 3.f;
    float inner = area.getWidth() - pad * 2.f;

    // PEAK
    float peakH = 9.f;
    juce::Rectangle<float> peakR(area.getX() + pad, area.getY() + pad, inner, peakH);
    drawLEDSegment(g, peakR, GC2020Colors::ledRed, isPeak);
    g.setFont(juce::Font("Arial", 6.5f, juce::Font::bold));
    g.setColour(isPeak ? GC2020Colors::ledRed.brighter(0.3f) : GC2020Colors::textDim);
    g.drawText("PK", peakR, juce::Justification::centred, false);

    // 5 segmentos GR
    struct Seg { float thr; juce::Colour col; const char* lbl; };
    static const Seg segs[] = {
        {  0.f, GC2020Colors::ledGreen,   " 0" },
        {  4.f, GC2020Colors::ledGreen,   "-4" },
        {  8.f, GC2020Colors::ledYellow,  "-8" },
        { 16.f, GC2020Colors::ledOrange, "-16" },
        { 24.f, GC2020Colors::ledRed,    "-24" },
    };

    float topY   = area.getY() + pad + peakH + 4.f;
    float totalH = area.getBottom() - pad - topY;
    float segH   = totalH / 5.f;
    float gap    = 2.2f;

    for (int i = 0; i < 5; ++i)
    {
        bool on = gainReductionDB >= segs[i].thr;
        juce::Rectangle<float> sr(area.getX() + pad,
                                   topY + i * segH + gap,
                                   inner, segH - gap);
        drawLEDSegment(g, sr, segs[i].col, on);

        g.setFont(juce::Font("Arial", 7.f, juce::Font::plain));
        g.setColour(on ? segs[i].col.withAlpha(0.9f) : GC2020Colors::textDim);
        g.drawText(segs[i].lbl, -24, (int)(topY + i * segH + gap),
                   22, (int)(segH - gap), juce::Justification::centredRight, false);
    }

    g.setFont(juce::Font("Arial", 7.f, juce::Font::bold));
    g.setColour(GC2020Colors::textDim);
    g.drawText("GR", (int)area.getX(), (int)area.getBottom() + 1,
               (int)area.getWidth(), 10, juce::Justification::centred, false);
}

// ═════════════════════════════════════════════════════════════════════════════
// ChannelStrip
// ═════════════════════════════════════════════════════════════════════════════
ChannelStrip::ChannelStrip(bool isLeft_, juce::AudioProcessorValueTreeState& apvts,
                           ValueTooltip& tooltip)
    : isLeft(isLeft_), pfx(isLeft_ ? "ch1_" : "ch2_"), tooltipRef(tooltip)
{
    auto setup = [&](juce::Slider& s, const juce::String& id, std::unique_ptr<SA>& att)
    {
        setupKnob(s, pfx + id, apvts, att);
    };

    setup(gateKnob,    "gate",      gateAtt);
    setup(threshKnob,  "threshold", threshAtt);
    setup(ratioKnob,   "ratio",     ratioAtt);
    setup(attackKnob,  "attack",    attackAtt);
    setup(releaseKnob, "release",   releaseAtt);
    setup(inputKnob,   "input",     inputAtt);
    setup(outputKnob,  "output",    outputAtt);

    compBtn.setLookAndFeel(&laf);
    compBtn.setClickingTogglesState(true);
    compBtn.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(compBtn);
    compAtt = std::make_unique<BA>(apvts, pfx + "comp_on", compBtn);

    addAndMakeVisible(grMeter);
}

ChannelStrip::~ChannelStrip()
{
    for (auto* s : { &gateKnob, &threshKnob, &ratioKnob,
                     &attackKnob, &releaseKnob, &inputKnob, &outputKnob })
        s->setLookAndFeel(nullptr);
    compBtn.setLookAndFeel(nullptr);
}

void ChannelStrip::setupKnob(juce::Slider& s, const juce::String& paramID,
                              juce::AudioProcessorValueTreeState& apvts,
                              std::unique_ptr<SA>& att)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setLookAndFeel(&laf);

    // Tooltip al cambiar valor
    s.onValueChange = [&s, this]()
    {
        auto* parent = getParentComponent();
        if (parent == nullptr) return;

        juce::String unit = "";
        if (&s == &attackKnob || &s == &releaseKnob) unit = " ms";
        else if (&s == &threshKnob || &s == &gateKnob) unit = " dB";

        juce::String txt = juce::String(s.getValue(), 1) + unit;
        auto pos = s.localPointToGlobal(s.getBounds().getCentre());
        auto rel = parent->getLocalPoint(nullptr, pos);
        tooltipRef.show(txt, rel);
    };

    addAndMakeVisible(s);
    att = std::make_unique<SA>(apvts, paramID, s);
}

void ChannelStrip::updateMeters(float grDB, bool gateClosed, bool peak, float pulse)
{
    gateActive = gateClosed;
    grMeter.setGR(grDB);
    grMeter.setPeak(peak);
    grMeter.setPulse(pulse);
    repaint();
}

void ChannelStrip::resized()
{
    auto area = getLocalBounds().reduced(4, 2);
    int  h    = area.getHeight();
    int  ky   = area.getY() + (h - kKnobTotal) / 2 + 2;
    int  ks   = kKnobSize;
    int  gap  = kKnobGap;
    int  x    = area.getX();

    gateKnob.setBounds   (x, ky, ks, ks); x += ks + gap;
    threshKnob.setBounds (x, ky, ks, ks); x += ks + gap;
    ratioKnob.setBounds  (x, ky, ks, ks); x += ks + gap;
    attackKnob.setBounds (x, ky, ks, ks); x += ks + gap;
    releaseKnob.setBounds(x, ky, ks, ks); x += ks + gap + 6;

    int btnW = ks, btnH = ks - 4;
    compBtn.setBounds(x, ky + (ks - btnH) / 2, btnW, btnH);
    x += btnW + gap + 6;

    int meterW = 26;
    grMeter.setBounds(x, area.getY() + 4, meterW, h - 8);
    x += meterW + gap + 10;

    inputKnob.setBounds (x, ky, ks, ks); x += ks + gap;
    outputKnob.setBounds(x, ky, ks, ks);
}

// ─────────────────────────────────────────────────────────────────────────────
void ChannelStrip::drawLED(juce::Graphics& g, juce::Rectangle<float> b,
                            juce::Colour col, bool on, float pulse)
{
    g.setColour(juce::Colour(0xff060606));
    g.fillEllipse(b);

    if (on)
    {
        g.setColour(col.withAlpha(0.38f * pulse));
        g.fillEllipse(b.expanded(4.f));
    }

    juce::ColourGradient lg(on ? col.brighter(0.4f * pulse) : col.darker(3.5f),
                             b.getX(), b.getY(),
                             col.darker(0.5f), b.getRight(), b.getBottom(), false);
    g.setGradientFill(lg);
    g.fillEllipse(b);

    if (on)
    {
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.fillEllipse(b.reduced(b.getWidth() * 0.28f)
                       .translated(-b.getWidth() * 0.1f, -b.getHeight() * 0.15f));
    }

    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.drawEllipse(b, 0.7f);
}

// ─────────────────────────────────────────────────────────────────────────────
void ChannelStrip::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    int  h    = getLocalBounds().getHeight();
    int  ky   = getLocalBounds().getY() + (h - kKnobTotal) / 2 + 2;

    // Panel de fondo con bisel 3D
    {
        juce::ColourGradient bg(juce::Colour(0xff1c1c1c), area.getX(), area.getY(),
                                 juce::Colour(0xff0d0d0d), area.getX(), area.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(area.reduced(1.f), 4.f);
        g.setColour(juce::Colour(0xff060606));
        g.drawRoundedRectangle(area.reduced(1.f), 4.f, 1.f);
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawRoundedRectangle(area.reduced(2.f), 3.f, 0.5f);
    }

    // Pill del canal
    {
        juce::Rectangle<float> pill(area.getX() + 5.f, area.getY() + 3.f, 38.f, 12.f);
        g.setColour(GC2020Colors::yamahaGold.withAlpha(0.14f));
        g.fillRoundedRectangle(pill, 3.f);
        g.setColour(GC2020Colors::yamahaGold.withAlpha(0.4f));
        g.drawRoundedRectangle(pill, 3.f, 0.5f);
        g.setFont(juce::Font("Arial", 8.5f, juce::Font::bold));
        g.setColour(GC2020Colors::yamahaGold);
        g.drawText(isLeft ? "CH 1" : "CH 2", pill, juce::Justification::centred, false);
    }

    // Etiquetas de knobs
    struct KL { juce::Slider* s; const char* txt; };
    KL labels[] = {
        { &gateKnob,    "EXP GATE"   },
        { &threshKnob,  "THRESHOLD"  },
        { &ratioKnob,   "COMP RATIO" },
        { &attackKnob,  "ATTACK"     },
        { &releaseKnob, "RELEASE"    },
        { &inputKnob,   "INPUT"      },
        { &outputKnob,  "OUTPUT"     },
    };

    g.setFont(juce::Font("Arial", 7.2f, juce::Font::bold));
    for (auto& lbl : labels)
    {
        auto kb = lbl.s->getBounds();
        g.setColour(GC2020Colors::textLabel);
        g.drawText(lbl.txt,
                   kb.getX() - 4, kb.getBottom() + 1,
                   kb.getWidth() + 8, kLabelH,
                   juce::Justification::centred, false);
    }

    // LED EXP GATE con efecto de respiración
    {
        auto kb = gateKnob.getBounds();
        juce::Rectangle<float> gl(kb.getCentreX() - 4.f,
                                   (float)(ky + kKnobSize + kLabelH + 4.f),
                                   8.f, 8.f);
        drawLED(g, gl, GC2020Colors::ledOrange, gateActive);
    }

    // Etiqueta COMP
    {
        auto cb = compBtn.getBounds();
        g.setFont(juce::Font("Arial", 6.5f, juce::Font::bold));
        g.setColour(GC2020Colors::textDim);
        g.drawText("COMP", cb.getX() - 2, cb.getBottom() + 1,
                   cb.getWidth() + 4, 10, juce::Justification::centred, false);
    }

    // Separador y etiqueta I/O
    {
        auto ib = inputKnob.getBounds();
        auto ob = outputKnob.getBounds();
        int  sx = (ib.getRight() + ob.getX()) / 2;
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawVerticalLine(sx, (float)(ky - 4), (float)(ky + kKnobSize + kLabelH + 4));
        g.setFont(juce::Font("Arial", 6.5f, juce::Font::plain));
        g.setColour(GC2020Colors::textDim.withAlpha(0.6f));
        g.drawText("I/O", sx - 10, ky - 12, 20, 9, juce::Justification::centred, false);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Editor principal
// ═════════════════════════════════════════════════════════════════════════════
YamahaGC2020BAudioProcessorEditor::YamahaGC2020BAudioProcessorEditor(
    YamahaGC2020BAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p),
      chL(true,  p.apvts, tooltip),
      chR(false, p.apvts, tooltip)
{
    setSize(960, 112);

    addAndMakeVisible(chL);
    addAndMakeVisible(chR);

    linkBtn.setLookAndFeel(&laf);
    linkBtn.setClickingTogglesState(true);
    addAndMakeVisible(linkBtn);
    linkAtt = std::make_unique<BA>(p.apvts, "link_mode", linkBtn);

    // Tooltip siempre encima
    addAndMakeVisible(tooltip);
    tooltip.setAlwaysOnTop(true);

    // 60 fps para animaciones fluidas
    startTimerHz(60);
}

YamahaGC2020BAudioProcessorEditor::~YamahaGC2020BAudioProcessorEditor()
{
    stopTimer();
    linkBtn.setLookAndFeel(nullptr);
}

void YamahaGC2020BAudioProcessorEditor::timerCallback()
{
    // ── Animación de respiración de LEDs (60fps) ──────────────────────────────
    ledPulse += 0.05f;
    if (ledPulse > juce::MathConstants<float>::twoPi)
        ledPulse -= juce::MathConstants<float>::twoPi;

    float pulseVal = 1.f + std::sin(ledPulse) * 0.12f;

    // ── Parpadeo sutil del Power LED (~1 vez cada 8 seg) ─────────────────────
    ++blinkCounter;
    if (blinkCounter > 480) // 8 seg a 60fps
    {
        blinkCounter = 0;
        powerBlink = !powerBlink;
    }

    // ── Offset del fondo animado ──────────────────────────────────────────────
    bgOffset += 0.003f;
    if (bgOffset > 1.f) bgOffset = 0.f;

    // ── Actualiza meters con pulse ────────────────────────────────────────────
    chL.updateMeters(proc.channelL.gainReductionDB.load(),
                     proc.channelL.gateClosed.load(),
                     proc.channelL.peakClip.load(),
                     pulseVal);

    chR.updateMeters(proc.channelR.gainReductionDB.load(),
                     proc.channelR.gateClosed.load(),
                     proc.channelR.peakClip.load(),
                     pulseVal);

    repaint();
}

void YamahaGC2020BAudioProcessorEditor::resized()
{
    auto area  = getLocalBounds();
    int  logoW = 80;
    int  linkW = 42;
    int  earW  = 10;
    int  half  = (area.getWidth() - logoW - linkW - earW * 2) / 2;

    chL.setBounds(logoW, 3, half, area.getHeight() - 6);

    int midX = logoW + half;
    linkBtn.setBounds(midX, area.getCentreY() - 18, linkW, 36);

    chR.setBounds(midX + linkW, 3, half, area.getHeight() - 6);

    tooltip.setBounds(0, 0, getWidth(), getHeight());
}

// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessorEditor::drawScrew(juce::Graphics& g,
                                                    float x, float y)
{
    float r = 5.f;
    // Sombra
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(x - r, y - r + 1.f, r * 2.f, r * 2.f);

    // Cuerpo
    juce::ColourGradient sg(juce::Colour(0xff606060), x - r * 0.4f, y - r * 0.5f,
                             juce::Colour(0xff181818), x + r, y + r, false);
    g.setGradientFill(sg);
    g.fillEllipse(x - r, y - r, r * 2.f, r * 2.f);

    // Borde oscuro
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawEllipse(x - r, y - r, r * 2.f, r * 2.f, 0.8f);

    // Ranura Phillips
    g.setColour(juce::Colour(0xff0a0a0a));
    g.drawLine(x - r * 0.52f, y, x + r * 0.52f, y, 1.2f);
    g.drawLine(x, y - r * 0.52f, x, y + r * 0.52f, 1.2f);

    // Especular
    g.setColour(juce::Colours::white.withAlpha(0.22f));
    g.fillEllipse(x - r * 0.45f, y - r * 0.6f, r * 0.55f, r * 0.38f);
}

// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessorEditor::drawEar(juce::Graphics& g,
                                                  float ex, float ew, float h)
{
    juce::ColourGradient eg(juce::Colour(0xff252525), ex, 0.f,
                             juce::Colour(0xff111111), ex + ew, h, false);
    g.setGradientFill(eg);
    g.fillRect(ex, 0.f, ew, h);

    // Separador
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawVerticalLine((int)(ex + ew - 0.5f), 4.f, h - 4.f);
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawVerticalLine((int)(ex + ew), 4.f, h - 4.f);
}

// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessorEditor::paint(juce::Graphics& g)
{
    drawBackground(g);

    
}


void YamahaGC2020BAudioProcessorEditor::drawBackground(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float w = b.getWidth(), h = b.getHeight();

    // ── Chasis negro satinado ─────────────────────────────────────────────────
    juce::ColourGradient bg(juce::Colour(0xff1e1e1e), 0.f, 0.f,
                             juce::Colour(0xff0c0c0c), 0.f, h, false);
    g.setGradientFill(bg);
    g.fillRect(b);

    // ── Textura brushed metal procedural con animación sutil ─────────────────
    for (float y = 0.f; y < h; y += 1.5f)
    {
        float noise = std::sin((y + bgOffset * 3.f) * 0.8f) * 0.5f + 0.5f;
        float a = 0.012f + noise * 0.010f;
        g.setColour(juce::Colours::white.withAlpha(a));
        g.drawHorizontalLine((int)y, 0.f, w);
    }

    // ── Ribete superior 3D ────────────────────────────────────────────────────
    {
        juce::ColourGradient tr(juce::Colour(0xffcccccc), 0.f, 0.f,
                                 juce::Colour(0xff282828), 0.f, 4.f, false);
        g.setGradientFill(tr);
        g.fillRect(0.f, 0.f, w, 4.f);
    }
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawHorizontalLine(4, 0.f, w);

    // ── Ribete inferior ───────────────────────────────────────────────────────
    {
        juce::ColourGradient br2(juce::Colour(0xff4a4a4a), 0.f, h - 4.f,
                                  juce::Colour(0xff0e0e0e), 0.f, h, false);
        g.setGradientFill(br2);
        g.fillRect(0.f, h - 4.f, w, 4.f);
    }
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawHorizontalLine((int)(h - 4.f), 0.f, w);

    // ── Rack ears ─────────────────────────────────────────────────────────────
    drawEar(g, 0.f,      10.f, h);
    drawEar(g, w - 10.f, 10.f, h);

    // ── Panel del logo ────────────────────────────────────────────────────────
    {
        juce::ColourGradient lg(juce::Colour(0xff1e1e1e), 11.f, 4.f,
                                 juce::Colour(0xff111111), 78.f, h - 4.f, false);
        g.setGradientFill(lg);
        g.fillRoundedRectangle(11.f, 4.f, 68.f, h - 8.f, 3.f);
        g.setColour(juce::Colour(0xff2e2e2e));
        g.drawRoundedRectangle(11.f, 4.f, 68.f, h - 8.f, 3.f, 0.7f);
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.drawRoundedRectangle(12.f, 5.f, 66.f, h - 10.f, 2.5f, 0.5f);
    }

    // Logo YAMAHA con anti-alias premium
    g.setFont(juce::Font("Arial", 12.f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("LLAMAJA", 13, 7, 64, 13, juce::Justification::centred, false);

    // Línea dorada
    {
        juce::ColourGradient lg(GC2020Colors::yamahaGold, 16.f, 22.f,
                                 GC2020Colors::goldDim, 74.f, 22.f, false);
        g.setGradientFill(lg);
        g.fillRect(16.f, 22.f, 58.f, 1.2f);
    }

    // Modelo
    g.setFont(juce::Font("Arial", 8.5f, juce::Font::bold));
    g.setColour(GC2020Colors::yamahaGold);
    g.drawText("GC2020B II", 13, 25, 64, 11, juce::Justification::centred, false);

    // Descripción
    g.setFont(juce::Font("Arial", 6.f, juce::Font::plain));
    g.setColour(juce::Colours::lightgrey.withAlpha(0.45f));
    g.drawText("STEREO COMP/LIMITER", 13, 38, 64, 8,
               juce::Justification::centred, false);

    // ── Power LED con parpadeo ────────────────────────────────────────────────
    float plx      = 45.f;
    float ply      = h * 0.68f;   // ← centrado en el panel, lejos del ribete
    float plr      = 4.5f;
    float pwrAlpha = powerBlink ? 1.f : 0.65f;

    // Halo
    g.setColour(GC2020Colors::ledGreen.withAlpha(0.28f * pwrAlpha));
    g.fillEllipse(plx - plr - 3.f, ply - plr - 3.f,
                  (plr + 3.f) * 2.f, (plr + 3.f) * 2.f);

    // Cuerpo
    {
        juce::ColourGradient pwrG(
            GC2020Colors::ledGreen.brighter(0.5f * pwrAlpha), plx - plr, ply - plr,
            GC2020Colors::ledGreen.darker(0.4f),               plx + plr, ply + plr, false);
        g.setGradientFill(pwrG);
    }
    g.fillEllipse(plx - plr, ply - plr, plr * 2.f, plr * 2.f);
    
    // Especular
    g.setColour(juce::Colours::white.withAlpha(0.5f * pwrAlpha));
    g.fillEllipse(plx - plr * 0.55f, ply - plr * 0.7f, plr * 0.8f, plr * 0.5f);

    // Etiqueta POWER debajo del LED
    g.setFont(juce::Font("Arial", 6.f, juce::Font::plain));
    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.drawText("POWER", 18, (int)(ply + plr + 3.f), 54, 9,
               juce::Justification::centred, false);

    // Etiqueta COMP/LIMIT debajo de POWER
    g.setFont(juce::Font("Arial", 7.5f, juce::Font::plain));
    g.setColour(GC2020Colors::textDim.withAlpha(0.65f));
    g.drawText("  By David Signals", 14, (int)(ply + plr + 13.f), 62, 8,
               juce::Justification::centred, false);
               
    // ── Separadores zona LINK ─────────────────────────────────────────────────
    int logoW = 80, linkW = 42, earW = 10;
    int half  = (int)(w - logoW - linkW - earW * 2) / 2;
    float lx1 = (float)(logoW + half);
    float lx2 = lx1 + linkW;

    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.drawVerticalLine((int)lx1,     5.f, h - 5.f);
    g.drawVerticalLine((int)lx2,     5.f, h - 5.f);
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawVerticalLine((int)lx1 + 1, 5.f, h - 5.f);
    g.drawVerticalLine((int)lx2 + 1, 5.f, h - 5.f);

    g.setFont(juce::Font("Arial", 7.f, juce::Font::bold));
    g.setColour(GC2020Colors::textDim);
    g.drawText("LINK", (int)lx1, (int)(h - 10.f), linkW, 8,
               juce::Justification::centred, false);

    // ── Tornillos ─────────────────────────────────────────────────────────────
    drawScrew(g, 5.f,     h * 0.5f);
    drawScrew(g, w - 5.f, h * 0.5f);
    

}

