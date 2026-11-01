#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ═════════════════════════════════════════════════════════════════════════════
// Paleta de colores premium
// ═════════════════════════════════════════════════════════════════════════════
namespace GC2020Colors
{
    const juce::Colour chassis      { 0xff0e0e0e };
    const juce::Colour panelBg      { 0xff141414 };
    const juce::Colour metalMid     { 0xff2a2a2a };
    const juce::Colour rimSilver    { 0xff909090 };
    const juce::Colour rimDark      { 0xff333333 };
    const juce::Colour yamahaGold   { 0xffC8960C };
    const juce::Colour goldDim      { 0xff7a5a08 };
    const juce::Colour textWhite    { 0xffEEEEEE };
    const juce::Colour textLabel    { 0xff999999 };
    const juce::Colour textDim      { 0xff555555 };
    const juce::Colour ledGreen     { 0xff00ee44 };
    const juce::Colour ledGreenDim  { 0xff002a0e };
    const juce::Colour ledYellow    { 0xffEECC00 };
    const juce::Colour ledYellowDim { 0xff332a00 };
    const juce::Colour ledOrange    { 0xffFF6600 };
    const juce::Colour ledOrangeDim { 0xff331400 };
    const juce::Colour ledRed       { 0xffFF1100 };
    const juce::Colour ledRedDim    { 0xff2a0400 };
    const juce::Colour knobBody     { 0xff1a1a1a };
    const juce::Colour knobSkirt    { 0xff111111 };
    const juce::Colour arcActive    { 0xffC8960C };
    const juce::Colour arcBg        { 0xff303030 };
}

// ═════════════════════════════════════════════════════════════════════════════
// YamahaLAF — LookAndFeel con knobs y botones premium
// ═════════════════════════════════════════════════════════════════════════════
class YamahaLAF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float pos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
};

// ═════════════════════════════════════════════════════════════════════════════
// GRMeter — Meter de Gain Reduction con 5 LEDs + PEAK
// ═════════════════════════════════════════════════════════════════════════════
class GRMeter : public juce::Component
{
public:
    void paint(juce::Graphics& g) override;
    void setGR(float db)  { gainReductionDB = db; repaint(); }
    void setPeak(bool p)  { isPeak = p;            repaint(); }
    void setPulse(float p){ ledPulseAlpha = p;     repaint(); }

private:
    float gainReductionDB = 0.f;
    float ledPulseAlpha   = 1.f;
    bool  isPeak          = false;
    void  drawLEDSegment(juce::Graphics& g, juce::Rectangle<float> b,
                         juce::Colour col, bool on);
};

// ═════════════════════════════════════════════════════════════════════════════
// ValueTooltip — Componente flotante que muestra el valor del knob
// ═════════════════════════════════════════════════════════════════════════════
class ValueTooltip : public juce::Component,
                     private juce::Timer
{
public:
    ValueTooltip();
    void show(const juce::String& text, juce::Point<int> pos);
    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    juce::String currentText;
    float        alpha = 0.f;
};

// ═════════════════════════════════════════════════════════════════════════════
// ChannelStrip — Canal completo con todos sus controles
// ═════════════════════════════════════════════════════════════════════════════
class ChannelStrip : public juce::Component
{
public:
    ChannelStrip(bool isLeft, juce::AudioProcessorValueTreeState& apvts,
                 ValueTooltip& tooltip);
    ~ChannelStrip() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters(float grDB, bool gateClosed, bool peak, float pulse);

    GRMeter grMeter;

private:
    bool         isLeft;
    juce::String pfx;
    YamahaLAF    laf;
    ValueTooltip& tooltipRef;

    juce::Slider gateKnob, threshKnob, ratioKnob,
                 attackKnob, releaseKnob, inputKnob, outputKnob;
    juce::TextButton compBtn { "IN" };
    bool gateActive = false;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> gateAtt, threshAtt, ratioAtt,
                        attackAtt, releaseAtt, inputAtt, outputAtt;
    std::unique_ptr<BA> compAtt;

    void setupKnob(juce::Slider& s, const juce::String& paramID,
                   juce::AudioProcessorValueTreeState& apvts,
                   std::unique_ptr<SA>& att);
    void drawLED(juce::Graphics& g, juce::Rectangle<float> b,
                 juce::Colour col, bool on, float pulse = 1.f);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};

// ═════════════════════════════════════════════════════════════════════════════
// Editor principal — 960 × 112 px rack 1U
// ═════════════════════════════════════════════════════════════════════════════
class YamahaGC2020BAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit YamahaGC2020BAudioProcessorEditor(YamahaGC2020BAudioProcessor&);
    ~YamahaGC2020BAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawBackground(juce::Graphics& g);
    void drawScrew(juce::Graphics& g, float x, float y);
    void drawEar(juce::Graphics& g, float ex, float ew, float h);

    YamahaGC2020BAudioProcessor& proc;
    YamahaLAF    laf;
    ValueTooltip tooltip;

    ChannelStrip chL, chR;

    juce::TextButton linkBtn { "LINK" };
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BA> linkAtt;

    // Animaciones
    float ledPulse      = 0.f;  // fase de respiración de LEDs
    float bgOffset      = 0.f;  // offset animado del fondo
    bool  powerBlink    = true; // estado parpadeo del power LED
    int   blinkCounter  = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YamahaGC2020BAudioProcessorEditor)
};
