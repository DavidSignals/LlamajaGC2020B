#pragma once
#include <JuceHeader.h>
#include "CompressorChannel.h"

// ─────────────────────────────────────────────────────────────────────────────
// YamahaGC2020BAudioProcessor: procesador principal del plugin
// ─────────────────────────────────────────────────────────────────────────────
class YamahaGC2020BAudioProcessor : public juce::AudioProcessor
{
public:
    YamahaGC2020BAudioProcessor();
    ~YamahaGC2020BAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "YamahaGC2020B"; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ── APVTS ───────────────────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Canales DSP ─────────────────────────────────────────────────────────
    CompressorChannel channelL;
    CompressorChannel channelR;

private:
    // Mapea knob 0-10 a dB (-20 a +20)
    float knobToDB(float knobVal) const { return (knobVal - 5.f) * 4.f; }

    // Actualiza parámetros DSP desde APVTS
    void updateChannelParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YamahaGC2020BAudioProcessor)
};
