#include "PluginEditor.h"

//==============================================================================
FIDICompEditor::FIDICompEditor(FIDICompProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      gainReductionMeter(p.getGainReduction()),
      thresholdAttachment(p.getAPVTS(), "threshold", thresholdSlider),
      ratioAttachment(p.getAPVTS(), "ratio", ratioSlider),
      attackAttachment(p.getAPVTS(), "attack", attackSlider),
      releaseAttachment(p.getAPVTS(), "release", releaseSlider),
      kneeAttachment(p.getAPVTS(), "knee", kneeSlider),
      makeupAttachment(p.getAPVTS(), "makeup", makeupSlider),
      mixAttachment(p.getAPVTS(), "mix", mixSlider)
{
    setLookAndFeel(&lookAndFeel);
    
    // Configure all sliders
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(ratioSlider, ratioLabel, "RATIO");
    setupSlider(attackSlider, attackLabel, "ATTACK");
    setupSlider(releaseSlider, releaseLabel, "RELEASE");
    setupSlider(kneeSlider, kneeLabel, "KNEE");
    setupSlider(makeupSlider, makeupLabel, "MAKEUP");
    setupSlider(mixSlider, mixLabel, "MIX");
    
    // Configure title label
    titleLabel.setText("FIDI COMP", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(28.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // Configure meter label
    meterLabel.setText("GR", juce::dontSendNotification);
    meterLabel.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    meterLabel.setColour(juce::Label::textColourId, juce::Colour(0x99ffffff));
    meterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(meterLabel);
    
    // Add meter
    addAndMakeVisible(gainReductionMeter);
    
    // Set window size - increased for better layout
    setSize(700, 340);
}

FIDICompEditor::~FIDICompEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void FIDICompEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 18);
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(slider);
    
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    label.setColour(juce::Label::textColourId, juce::Colour(0x99ffffff));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

//==============================================================================
void FIDICompEditor::paint(juce::Graphics& g)
{
    // Background - dark gradient
    juce::ColourGradient bgGradient(
        juce::Colour(0xff0d0d1a), 0.0f, 0.0f,
        juce::Colour(0xff1a1a2e), 0.0f, static_cast<float>(getHeight()),
        false);
    g.setGradientFill(bgGradient);
    g.fillAll();
    
    // Subtle noise texture effect via very subtle pattern
    g.setColour(juce::Colour(0x08ffffff));
    for (int i = 0; i < getWidth(); i += 3)
    {
        for (int j = 0; j < getHeight(); j += 3)
        {
            if ((i + j) % 6 == 0)
                g.fillRect(i, j, 1, 1);
        }
    }
    
    // Header section background
    g.setColour(juce::Colour(0x15ffffff));
    g.fillRect(0, 0, getWidth(), 55);
    
    // Header bottom border with glow
    juce::ColourGradient lineGradient(
        juce::Colour(0xff00d4ff), 20.0f, 55.0f,
        juce::Colour(0x0000d4ff), static_cast<float>(getWidth() - 20), 55.0f,
        false);
    g.setGradientFill(lineGradient);
    g.fillRect(20, 54, getWidth() - 40, 2);
    
    // Subtle glow under header line
    g.setColour(juce::Colour(0x2000d4ff));
    g.fillRect(20, 56, getWidth() - 40, 4);
    
    // Section labels
    g.setColour(juce::Colour(0x60ffffff));
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    
    // Upper row label
    g.drawText("COMPRESSION", 25, 70, 100, 14, juce::Justification::centredLeft);
    
    // Lower row label
    g.drawText("OUTPUT", 25, 200, 100, 14, juce::Justification::centredLeft);
    
    // Version tag
    g.setColour(juce::Colour(0x40ffffff));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("v1.0", getWidth() - 50, 20, 30, 14, juce::Justification::centredRight);
}

//==============================================================================
void FIDICompEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area
    titleLabel.setBounds(25, 14, 200, 30);
    
    // Meter on the right side
    int meterWidth = 35;
    int meterMargin = 20;
    auto meterArea = bounds.removeFromRight(meterWidth + meterMargin * 2);
    meterArea = meterArea.reduced(meterMargin, 0);
    meterArea.removeFromTop(70);
    meterArea.removeFromBottom(25);
    
    meterLabel.setBounds(meterArea.removeFromBottom(18));
    gainReductionMeter.setBounds(meterArea.reduced(0, 5));
    
    // Knob layout - 2 rows
    int knobSize = 75;
    int labelHeight = 16;
    int rowHeight = 115;
    int startY = 85;
    int leftMargin = 100;
    
    // Calculate columns for 5 knobs in first row
    int availableWidth = bounds.getWidth() - leftMargin - 20;
    int colWidth = availableWidth / 5;
    
    // Lambda to position a slider and label
    auto positionKnob = [&](juce::Slider& slider, juce::Label& label, int col, int row)
    {
        int x = leftMargin + col * colWidth + (colWidth - knobSize) / 2;
        int y = startY + row * rowHeight;
        
        label.setBounds(x, y, knobSize, labelHeight);
        slider.setBounds(x, y + labelHeight, knobSize, knobSize);
    };
    
    // Row 1: Compression controls (Threshold, Ratio, Attack, Release, Knee)
    positionKnob(thresholdSlider, thresholdLabel, 0, 0);
    positionKnob(ratioSlider, ratioLabel, 1, 0);
    positionKnob(attackSlider, attackLabel, 2, 0);
    positionKnob(releaseSlider, releaseLabel, 3, 0);
    positionKnob(kneeSlider, kneeLabel, 4, 0);
    
    // Row 2: Output controls (Makeup, Mix)
    positionKnob(makeupSlider, makeupLabel, 0, 1);
    positionKnob(mixSlider, mixLabel, 1, 1);
}
