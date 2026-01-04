#pragma once

#include <JuceHeader.h>

/**
 * Parameters class for FIDI Comp
 * Holds references to APVTS raw parameter values and converts them
 * to DSP-ready coefficients. Handles sample-rate aware calculations.
 */
class Parameters
{
public:
    //==============================================================================
    explicit Parameters(juce::AudioProcessorValueTreeState& apvts);

    //==============================================================================
    /** Set the sample rate for coefficient calculations */
    void setSampleRate(double newSampleRate);

    /** Update all DSP coefficients from current parameter values */
    void update();

    //==============================================================================
    // DSP-ready values (updated by update())
    
    float threshold = -20.0f;       // dB
    float ratio = 4.0f;             // :1
    float knee = 6.0f;              // dB
    float mix = 1.0f;               // 0.0 to 1.0
    float makeupLinear = 1.0f;      // Linear gain
    
    double attackCoeff = 0.0;       // One-pole attack coefficient
    double releaseCoeff = 0.0;      // One-pole release coefficient
    double smoothingCoeff = 0.0;    // Parameter smoothing coefficient

private:
    //==============================================================================
    /** Calculate one-pole filter coefficient from time in milliseconds */
    double calculateCoefficient(double timeMs) const;

    //==============================================================================
    double sampleRate = 44100.0;

    // Raw parameter references
    std::atomic<float>& thresholdParam;
    std::atomic<float>& ratioParam;
    std::atomic<float>& attackParam;
    std::atomic<float>& releaseParam;
    std::atomic<float>& kneeParam;
    std::atomic<float>& makeupParam;
    std::atomic<float>& mixParam;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)
};
