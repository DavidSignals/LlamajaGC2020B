#include "PluginProcessor.h"
#include "PluginEditor.h"   // ← CRÍTICO: necesario para createEditor()

// ─────────────────────────────────────────────────────────────────────────────
// createParameterLayout: define todos los parámetros del APVTS
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout
YamahaGC2020BAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto makeFloat = [](const juce::String& id, const juce::String& name,
                        float min, float max, float def)
    {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float>(min, max, 0.01f), def);
    };

    auto makeBool = [](const juce::String& id, const juce::String& name, bool def)
    {
        return std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { id, 1 }, name, def);
    };

    // ── Canal 1 ──────────────────────────────────────────────────────────────
    params.push_back(makeFloat("ch1_input",     "CH1 Input",      0.f,   10.f,   5.f));
    params.push_back(makeFloat("ch1_output",    "CH1 Output",     0.f,   10.f,   5.f));
    params.push_back(makeFloat("ch1_threshold", "CH1 Threshold", -35.f,  20.f, -10.f));
    params.push_back(makeFloat("ch1_ratio",     "CH1 Ratio",      1.f,   60.f,   4.f));
    params.push_back(makeFloat("ch1_attack",    "CH1 Attack",     0.1f,  50.f,  10.f));
    params.push_back(makeFloat("ch1_release",   "CH1 Release",   50.f, 2000.f, 200.f));
    params.push_back(makeFloat("ch1_gate",      "CH1 Gate",     -80.f,   0.f,  -80.f));
    params.push_back(makeBool ("ch1_comp_on",   "CH1 Comp On",   true));

    // ── Canal 2 ──────────────────────────────────────────────────────────────
    params.push_back(makeFloat("ch2_input",     "CH2 Input",      0.f,   10.f,   5.f));
    params.push_back(makeFloat("ch2_output",    "CH2 Output",     0.f,   10.f,   5.f));
    params.push_back(makeFloat("ch2_threshold", "CH2 Threshold", -35.f,  20.f, -10.f));
    params.push_back(makeFloat("ch2_ratio",     "CH2 Ratio",      1.f,   60.f,   4.f));
    params.push_back(makeFloat("ch2_attack",    "CH2 Attack",     0.1f,  50.f,  10.f));
    params.push_back(makeFloat("ch2_release",   "CH2 Release",   50.f, 2000.f, 200.f));
    params.push_back(makeFloat("ch2_gate",      "CH2 Gate",     -80.f,   0.f,  -80.f));
    params.push_back(makeBool ("ch2_comp_on",   "CH2 Comp On",   true));

    // ── Global ───────────────────────────────────────────────────────────────
    params.push_back(makeBool("link_mode", "Link Mode", false));

    return { params.begin(), params.end() };
}

// ─────────────────────────────────────────────────────────────────────────────
YamahaGC2020BAudioProcessor::YamahaGC2020BAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

YamahaGC2020BAudioProcessor::~YamahaGC2020BAudioProcessor() {}

// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    channelL.prepare(sampleRate);
    channelR.prepare(sampleRate);
}

void YamahaGC2020BAudioProcessor::releaseResources()
{
    channelL.reset();
    channelR.reset();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateChannelParams: sincroniza APVTS → canales DSP
// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessor::updateChannelParams()
{
    // ── Canal L ──────────────────────────────────────────────────────────────
    channelL.inputGainDB  = knobToDB(*apvts.getRawParameterValue("ch1_input"));
    channelL.outputGainDB = knobToDB(*apvts.getRawParameterValue("ch1_output"));
    channelL.thresholdDB  = *apvts.getRawParameterValue("ch1_threshold");
    channelL.ratio        = *apvts.getRawParameterValue("ch1_ratio");
    channelL.attackMs     = *apvts.getRawParameterValue("ch1_attack");
    channelL.releaseMs    = *apvts.getRawParameterValue("ch1_release");
    channelL.gateThreshDB = *apvts.getRawParameterValue("ch1_gate");
    channelL.compEnabled  = *apvts.getRawParameterValue("ch1_comp_on") > 0.5f;
    channelL.gateEnabled  = channelL.gateThreshDB > -79.f;

    // ── Canal R ──────────────────────────────────────────────────────────────
    bool linked = *apvts.getRawParameterValue("link_mode") > 0.5f;

    if (linked)
    {
        channelR.inputGainDB  = channelL.inputGainDB;
        channelR.outputGainDB = channelL.outputGainDB;
        channelR.thresholdDB  = channelL.thresholdDB;
        channelR.ratio        = channelL.ratio;
        channelR.attackMs     = channelL.attackMs;
        channelR.releaseMs    = channelL.releaseMs;
        channelR.gateThreshDB = channelL.gateThreshDB;
        channelR.compEnabled  = channelL.compEnabled;
        channelR.gateEnabled  = channelL.gateEnabled;
    }
    else
    {
        channelR.inputGainDB  = knobToDB(*apvts.getRawParameterValue("ch2_input"));
        channelR.outputGainDB = knobToDB(*apvts.getRawParameterValue("ch2_output"));
        channelR.thresholdDB  = *apvts.getRawParameterValue("ch2_threshold");
        channelR.ratio        = *apvts.getRawParameterValue("ch2_ratio");
        channelR.attackMs     = *apvts.getRawParameterValue("ch2_attack");
        channelR.releaseMs    = *apvts.getRawParameterValue("ch2_release");
        channelR.gateThreshDB = *apvts.getRawParameterValue("ch2_gate");
        channelR.compEnabled  = *apvts.getRawParameterValue("ch2_comp_on") > 0.5f;
        channelR.gateEnabled  = channelR.gateThreshDB > -79.f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// processBlock: bloque principal de procesamiento de audio
// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateChannelParams();

    auto* dataL = buffer.getWritePointer(0);
    auto* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    bool linked = *apvts.getRawParameterValue("link_mode") > 0.5f;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (linked)
        {
            // ── LINK MODE: aplica el mayor GR a ambos canales ────────────────
            float outL = channelL.processSample(dataL[i]);
            float outR = dataR ? channelR.processSample(dataR[i]) : 0.f;

            float grL  = channelL.gainReductionDB.load();
            float grR  = channelR.gainReductionDB.load();
            float maxGR = std::max(grL, grR);

            if (maxGR > grL + 0.01f)
                outL *= std::pow(10.f, -(maxGR - grL) / 20.f);

            if (dataR && maxGR > grR + 0.01f)
                outR *= std::pow(10.f, -(maxGR - grR) / 20.f);

            dataL[i] = outL;
            if (dataR) dataR[i] = outR;
        }
        else
        {
            // ── MODO INDEPENDIENTE ───────────────────────────────────────────
            dataL[i] = channelL.processSample(dataL[i]);
            if (dataR) dataR[i] = channelR.processSample(dataR[i]);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// createEditor: crea la instancia del editor visual
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorEditor* YamahaGC2020BAudioProcessor::createEditor()
{
    return new YamahaGC2020BAudioProcessorEditor(*this);
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado: guardar y cargar preset
// ─────────────────────────────────────────────────────────────────────────────
void YamahaGC2020BAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void YamahaGC2020BAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new YamahaGC2020BAudioProcessor();
}
