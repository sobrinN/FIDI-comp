#pragma once

#include "Parameters.h"

/**
 * Compressor DSP class for FIDI Comp
 * Implements envelope following and gain computation with soft knee.
 * This class is designed to be lightweight and efficient for real-time processing.
 */
class Compressor
{
public:
    //==============================================================================
    explicit Compressor(const Parameters& params);

    //==============================================================================
    /** Reset the compressor state (call when sample rate changes or playback starts) */
    void reset() noexcept;

    /**
     * Compute gain reduction for a given input level.
     * @param inputLevel Absolute value of input sample (or linked level for stereo)
     * @return Gain multiplier to apply (1.0 = no reduction, 0.5 = -6dB reduction)
     */
    [[nodiscard]] float computeGainReduction(float inputLevel) noexcept;

private:
    //==============================================================================
    /**
     * Compute gain reduction in dB using soft knee algorithm.
     * @param inputDb Input level in dB
     * @return Gain reduction in dB (positive value)
     */
    [[nodiscard]] float computeGainReductionDb(float inputDb) const noexcept;

    //==============================================================================
    const Parameters& parameters;

    // Envelope follower state
    double envelope = 0.0;

    // Smoothed parameter values (to prevent zipper noise)
    float smoothedThreshold = -20.0f;
    float smoothedRatio = 4.0f;
    float smoothedKnee = 6.0f;
    float smoothedMix = 1.0f;
    float smoothedMakeup = 1.0f;
    double smoothedAttackCoeff = 0.01;
    double smoothedReleaseCoeff = 0.001;
    
    // Batch smoothing: update every N samples for efficiency
    static constexpr int smoothingInterval = 32;
    int smoothingCounter = 0;

public:
    /** Get smoothed mix value (0-1) - call once per sample after computeGainReduction */
    [[nodiscard]] float getSmoothedMix() const noexcept { return smoothedMix; }
    
    /** Get smoothed makeup gain (linear) - call once per sample after computeGainReduction */
    [[nodiscard]] float getSmoothedMakeup() const noexcept { return smoothedMakeup; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Compressor)
};
