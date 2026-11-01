#include "CompressorChannel.h"

// ─────────────────────────────────────────────────────────────────────────────
CompressorChannel::CompressorChannel()
{
    prepare(44100.0);
}

// ─────────────────────────────────────────────────────────────────────────────
void CompressorChannel::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;

    // Recalcula coeficientes con la nueva frecuencia de muestreo
    attackCoeff       = calcCoeff(attackMs);
    releaseCoeff      = calcCoeff(releaseMs);
    gateAttackCoeff   = calcCoeff(1.f);   // 1 ms fijo para gate
    gateReleaseCoeff  = calcCoeff(50.f);  // 50 ms fijo para gate

    // Recalcula hold del PEAK LED (~300 ms)
    kPeakHoldSamples = static_cast<int>(sampleRate * 0.3);

    reset();
}

// ─────────────────────────────────────────────────────────────────────────────
void CompressorChannel::reset()
{
    envelope        = -96.f;
    grSmooth        = 0.f;
    gateEnvelope    = -96.f;
    peakHoldCounter = 0;

    gainReductionDB.store(0.f);
    outputLevelDB.store(-96.f);
    gateClosed.store(false);
    peakClip.store(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// calcCoeff: coeficiente de suavizado de primer orden
//   coeff = exp(-1 / (sampleRate * timeMs / 1000))
// ─────────────────────────────────────────────────────────────────────────────
float CompressorChannel::calcCoeff(float timeMs) const
{
    if (timeMs <= 0.f) return 0.f;
    return std::exp(-1.f / (static_cast<float>(sampleRate) * timeMs / 1000.f));
}

// ─────────────────────────────────────────────────────────────────────────────
// processSample: cadena DSP completa muestra a muestra
// ─────────────────────────────────────────────────────────────────────────────
float CompressorChannel::processSample(float input)
{
    // ── 1. Ganancia de entrada ──────────────────────────────────────────────
    float sample = input * dBToLinear(inputGainDB);

    // ── 2. Nivel en dB para el detector ─────────────────────────────────────
    float levelDB = linearToDB(sample);

    // ── 3. Recalcular coeficientes ballistics ────────────────────────────────
    attackCoeff  = calcCoeff(attackMs);
    releaseCoeff = calcCoeff(releaseMs);

    // ── 4. Detector de envolvente con ballistics (peak, log-domain) ──────────
    if (levelDB > envelope)
        envelope = attackCoeff  * envelope + (1.f - attackCoeff)  * levelDB;
    else
        envelope = releaseCoeff * envelope + (1.f - releaseCoeff) * levelDB;

    // ── 5. Cálculo de gain reduction ─────────────────────────────────────────
    float gr = 0.f;

    if (compEnabled && ratio > 1.f)
    {
        float overshoot = envelope - thresholdDB;
        if (overshoot > 0.f)
        {
            // ratio >= 50 se considera infinito (limitador)
            float effectiveRatio = (ratio >= 50.f) ? 1000.f : ratio;
            gr = overshoot * (1.f - 1.f / effectiveRatio);
        }
    }

    // ── 6. Suavizado de la GR para evitar clicks ─────────────────────────────
    if (gr > grSmooth)
        grSmooth = attackCoeff  * grSmooth + (1.f - attackCoeff)  * gr;
    else
        grSmooth = releaseCoeff * grSmooth + (1.f - releaseCoeff) * gr;

    // ── 7. Aplicar compresión ────────────────────────────────────────────────
    if (compEnabled)
        sample *= dBToLinear(-grSmooth);

    // ── 8. Expander Gate ─────────────────────────────────────────────────────
    bool isGateClosed = false;

    if (gateEnabled && gateThreshDB > -79.f)
    {
        // Detector rápido para el gate
        if (levelDB > gateEnvelope)
            gateEnvelope = gateAttackCoeff  * gateEnvelope + (1.f - gateAttackCoeff)  * levelDB;
        else
            gateEnvelope = gateReleaseCoeff * gateEnvelope + (1.f - gateReleaseCoeff) * levelDB;

        if (gateEnvelope < gateThreshDB)
        {
            // Atenúa proporcionalmente (expander downward)
            float gateGainDB = (gateEnvelope - gateThreshDB) * 2.5f;
            sample *= dBToLinear(std::max(gateGainDB, -60.f));
            isGateClosed = true;
        }
    }

    // ── 9. Ganancia de salida ────────────────────────────────────────────────
    sample *= dBToLinear(outputGainDB);

    // ── 10. Actualizar métricas para la UI ───────────────────────────────────
    gainReductionDB.store(grSmooth);
    outputLevelDB.store(linearToDB(sample));
    gateClosed.store(isGateClosed);

    // PEAK LED: hold de ~300 ms
    if (std::abs(sample) >= 1.f)
    {
        peakHoldCounter = kPeakHoldSamples;
        peakClip.store(true);
    }
    else if (peakHoldCounter > 0)
    {
        --peakHoldCounter;
    }
    else
    {
        peakClip.store(false);
    }

    return sample;
}
