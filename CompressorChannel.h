#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <atomic>

// ─────────────────────────────────────────────────────────────────────────────
// CompressorChannel: módulo DSP feed-forward VCA para un canal del GC2020B II
// ─────────────────────────────────────────────────────────────────────────────
class CompressorChannel
{
public:
    CompressorChannel();

    // Inicializa con la frecuencia de muestreo actual
    void prepare(double newSampleRate);

    // Procesa una muestra individual; devuelve la muestra comprimida
    float processSample(float input);

    // Resetea envolventes
    void reset();

    // ── Parámetros del canal (escritos desde el hilo de audio) ──
    float inputGainDB   = 0.f;
    float outputGainDB  = 0.f;
    float thresholdDB   = -10.f;
    float ratio         = 4.f;
    float attackMs      = 10.f;
    float releaseMs     = 200.f;
    float gateThreshDB  = -80.f;
    bool  compEnabled   = true;
    bool  gateEnabled   = false;

    // ── Métricas para la UI (leídas con atomics desde el editor) ──
    std::atomic<float> gainReductionDB { 0.f };
    std::atomic<float> outputLevelDB   { -96.f };
    std::atomic<bool>  gateClosed      { false };
    std::atomic<bool>  peakClip        { false };

private:
    double sampleRate       = 44100.0;

    float envelope          = 0.f;
    float grSmooth          = 0.f;
    float gateEnvelope      = 0.f;

    float attackCoeff       = 0.f;
    float releaseCoeff      = 0.f;
    float gateAttackCoeff   = 0.f;
    float gateReleaseCoeff  = 0.f;

    int   peakHoldCounter   = 0;
    int   kPeakHoldSamples  = 13230;  // ~300 ms a 44100 Hz (sin constexpr)

    float calcCoeff(float timeMs) const;
    static float dBToLinear(float dB)  { return std::pow(10.f, dB / 20.f); }
    static float linearToDB(float lin) { return 20.f * std::log10(std::abs(lin) + 1e-9f); }
};
