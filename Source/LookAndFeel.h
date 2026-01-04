#pragma once

#include <JuceHeader.h>

/**
 * Custom LookAndFeel for FIDI Comp
 * Provides modern dark theme styling with glow effects.
 */
class FIDILookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==============================================================================
    FIDILookAndFeel();
    ~FIDILookAndFeel() override = default;

    //==============================================================================
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

private:
    //==============================================================================
    // Color palette - vibrant cyan accent
    juce::Colour backgroundColour{0xff0d0d1a};
    juce::Colour panelColour{0xff1a1a2e};
    juce::Colour accentColour{0xff00d4ff};
    juce::Colour accentLightColour{0xff80eaff};
    juce::Colour accentGlowColour{0x4000d4ff};
    juce::Colour textColour{0xffffffff};
    juce::Colour textDimColour{0x99ffffff};
    juce::Colour trackColour{0xff2a2a3e};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FIDILookAndFeel)
};
