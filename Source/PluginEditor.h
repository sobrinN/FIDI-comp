#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Meter.h"

/**
 * FIDI Comp Plugin Editor
 * Main GUI class with rotary knobs for all compression parameters
 * and real-time gain reduction metering.
 */
class FIDICompEditor : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    explicit FIDICompEditor(FIDICompProcessor&);
    ~FIDICompEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    /** Helper to create and configure a rotary slider */
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);

    //==============================================================================
    [[maybe_unused]] FIDICompProcessor& processorRef;
    FIDILookAndFeel lookAndFeel;
    
    // Meter
    Meter gainReductionMeter;
    
    // Sliders
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider kneeSlider;
    juce::Slider makeupSlider;
    juce::Slider mixSlider;
    
    // Labels
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label kneeLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;
    juce::Label titleLabel;
    juce::Label meterLabel;
    
    // Attachments (must be declared after sliders)
    juce::AudioProcessorValueTreeState::SliderAttachment thresholdAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment ratioAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment releaseAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment kneeAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment makeupAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment mixAttachment;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FIDICompEditor)
};
