#pragma once

#include <JuceHeader.h>

/**
 * Gain reduction meter component for FIDI Comp
 * Displays real-time compression activity with smoothed visualization.
 */
class Meter : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    explicit Meter(std::atomic<float>& gainReduction);
    ~Meter() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    std::atomic<float>& gainReductionAtomic;
    
    float displayValue = 0.0f;  // Current display value in dB (0 to maxDb)
    
    // Meter ballistics - calculated for 30fps timer rate
    float attackCoeff = 0.3f;    // Fast attack for responsiveness
    float releaseCoeff = 0.02f;  // Slow release for readability
    static constexpr float maxDb = 24.0f;         // Maximum display range in dB
    static constexpr int numSegments = 16;        // Number of LED segments
    static constexpr float timerRateHz = 30.0f;   // Timer refresh rate

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Meter)
};
