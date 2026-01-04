#include "Parameters.h"

//==============================================================================
Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
    : thresholdParam(*apvts.getRawParameterValue("threshold")),
      ratioParam(*apvts.getRawParameterValue("ratio")),
      attackParam(*apvts.getRawParameterValue("attack")),
      releaseParam(*apvts.getRawParameterValue("release")),
      kneeParam(*apvts.getRawParameterValue("knee")),
      makeupParam(*apvts.getRawParameterValue("makeup")),
      mixParam(*apvts.getRawParameterValue("mix"))
{
}

//==============================================================================
void Parameters::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    
    // Calculate smoothing coefficient for ~30ms smoothing time
    // Note: For smoothing we need (1 - exp) since we use it as a speed coefficient
    // Formula: smoothed += coeff * (target - smoothed)
    // This requires coeff close to 0 for slow smoothing, close to 1 for fast
    smoothingCoeff = 1.0 - std::exp(-1.0 / (sampleRate * 0.030));  // 30ms
    
    // Force update of all coefficients
    update();
}

//==============================================================================
double Parameters::calculateCoefficient(double timeMs) const
{
    if (timeMs <= 0.0 || sampleRate <= 0.0)
        return 0.0;
    
    // One-pole filter coefficient: exp(-1 / (sampleRate * timeSeconds))
    // This gives values close to 1.0 for slow (long times) and close to 0.0 for fast (short times)
    return std::exp(-1.0 / (sampleRate * timeMs * 0.001));
}

//==============================================================================
void Parameters::update()
{
    // Read raw parameters
    threshold = thresholdParam.load();
    ratio = ratioParam.load();
    knee = kneeParam.load();
    
    // Convert mix from percentage to 0-1 range
    mix = mixParam.load() * 0.01f;
    
    // Convert makeup from dB to linear gain
    float makeupDb = makeupParam.load();
    makeupLinear = juce::Decibels::decibelsToGain(makeupDb);
    
    // Calculate attack and release coefficients
    float attackMs = attackParam.load();
    float releaseMs = releaseParam.load();
    
    attackCoeff = calculateCoefficient(static_cast<double>(attackMs));
    releaseCoeff = calculateCoefficient(static_cast<double>(releaseMs));
}
