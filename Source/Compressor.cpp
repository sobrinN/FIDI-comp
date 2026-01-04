#include "Compressor.h"

//==============================================================================
Compressor::Compressor(const Parameters& params)
    : parameters(params)
{
}

//==============================================================================
void Compressor::reset()
{
    envelope = 0.0;
    smoothedThreshold = parameters.threshold;
    smoothedRatio = parameters.ratio;
    smoothedKnee = parameters.knee;
    smoothedMix = parameters.mix;
    smoothedMakeup = parameters.makeupLinear;
    smoothedAttackCoeff = parameters.attackCoeff;
    smoothedReleaseCoeff = parameters.releaseCoeff;
}

//==============================================================================
float Compressor::computeGainReduction(float inputLevel)
{
    // Smooth parameter changes to prevent zipper noise
    const float smoothCoeff = static_cast<float>(parameters.smoothingCoeff);
    smoothedThreshold += smoothCoeff * (parameters.threshold - smoothedThreshold);
    smoothedRatio += smoothCoeff * (parameters.ratio - smoothedRatio);
    smoothedKnee += smoothCoeff * (parameters.knee - smoothedKnee);
    smoothedMix += smoothCoeff * (parameters.mix - smoothedMix);
    smoothedMakeup += smoothCoeff * (parameters.makeupLinear - smoothedMakeup);
    
    // Also smooth attack and release coefficients
    smoothedAttackCoeff += smoothCoeff * (parameters.attackCoeff - smoothedAttackCoeff);
    smoothedReleaseCoeff += smoothCoeff * (parameters.releaseCoeff - smoothedReleaseCoeff);

    // Envelope follower operates in LINEAR domain (not dB!)
    // This matches the skill reference pattern
    double coeff = (inputLevel > envelope) ? smoothedAttackCoeff : smoothedReleaseCoeff;
    envelope = coeff * (envelope - inputLevel) + inputLevel;
    
    // Ensure envelope doesn't go negative
    if (envelope < 0.0)
        envelope = 0.0;

    // AFTER smoothing: convert envelope to dB for gain computation
    constexpr float minLevel = 1e-10f;
    float envelopeDb = juce::Decibels::gainToDecibels(
        static_cast<float>(std::max(envelope, static_cast<double>(minLevel))));

    // Compute gain reduction based on envelope (in dB domain)
    float gainReductionDb = computeGainReductionDb(envelopeDb);

    // Convert dB reduction to linear gain
    // gainReductionDb is positive (e.g., 6dB of reduction)
    // We need to return a multiplier < 1.0
    float gainReduction = juce::Decibels::decibelsToGain(-gainReductionDb);

    // Safety check: prevent NaN/Inf (can occur with extreme parameter values)
    if (std::isnan(gainReduction) || std::isinf(gainReduction))
        gainReduction = 1.0f;
    
    // Clamp to valid range [0.0, 1.0] for safety
    gainReduction = std::clamp(gainReduction, 0.0f, 1.0f);

    return gainReduction;
}

//==============================================================================
float Compressor::computeGainReductionDb(float inputDb) const
{
    const float threshold = smoothedThreshold;
    const float ratio = smoothedRatio;
    const float kneeWidth = smoothedKnee;
    const float halfKnee = kneeWidth * 0.5f;

    // Below knee region: no compression
    if (inputDb <= threshold - halfKnee)
    {
        return 0.0f;
    }

    // Above knee region: full compression
    if (inputDb >= threshold + halfKnee)
    {
        float overDb = inputDb - threshold;
        float gainReductionDb = overDb - (overDb / ratio);
        return gainReductionDb;
    }

    // Inside knee region: gradual transition using interpolated effective ratio
    // This is the correct soft knee algorithm from the skill reference
    float kneePosition = inputDb - (threshold - halfKnee);
    float kneeRatio = kneePosition / kneeWidth;  // 0 to 1 within knee
    
    // Linearly interpolate ratio from 1:1 to full ratio
    float effectiveRatio = 1.0f + (ratio - 1.0f) * kneeRatio;
    
    // Calculate gain reduction using interpolated ratio
    float overDb = kneePosition;  // Distance into knee region
    float gainReductionDb = overDb - (overDb / effectiveRatio);
    
    return gainReductionDb;
}
