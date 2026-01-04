#pragma once

#include <JuceHeader.h>
#include "Compressor.h"
#include "Parameters.h"

//==============================================================================
/**
 * FIDI Comp - Professional Dynamics Compressor
 * Main AudioProcessor class handling audio routing, state management,
 * and coordination between DSP and GUI components.
 */
class FIDICompProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    FIDICompProcessor();
    ~FIDICompProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Returns the APVTS for editor to create parameter attachments */
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    /** Returns atomic gain reduction value for metering (0.0 to 1.0, where 1.0 = no reduction) */
    std::atomic<float>& getGainReduction() { return gainReductionAtomic; }

private:
    //==============================================================================
    /** Creates the parameter layout for APVTS */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    Parameters parameters;
    Compressor compressorL;
    Compressor compressorR;
    
    /** Atomic gain reduction for thread-safe metering */
    std::atomic<float> gainReductionAtomic{1.0f};
    
    /** Identifier for XML state */
    static const juce::Identifier stateIdentifier;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FIDICompProcessor)
};
