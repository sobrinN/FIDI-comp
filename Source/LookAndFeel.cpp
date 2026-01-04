#include "LookAndFeel.h"

//==============================================================================
FIDILookAndFeel::FIDILookAndFeel()
{
    // Set default colours
    setColour(juce::Slider::rotarySliderFillColourId, accentColour);
    setColour(juce::Slider::rotarySliderOutlineColourId, trackColour);
    setColour(juce::Slider::thumbColourId, accentColour);
    setColour(juce::Slider::textBoxTextColourId, textColour);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    
    setColour(juce::Label::textColourId, textColour);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    
    setColour(juce::TextEditor::textColourId, textColour);
    setColour(juce::TextEditor::backgroundColourId, panelColour);
    setColour(juce::TextEditor::highlightColourId, accentColour);
    setColour(juce::TextEditor::highlightedTextColourId, textColour);
    setColour(juce::CaretComponent::caretColourId, accentColour);
}

//==============================================================================
void FIDILookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    juce::ignoreUnused(slider);
    
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw outer glow for value arc (subtle)
    if (sliderPos > 0.01f)
    {
        juce::Path glowArc;
        glowArc.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f,
                               0.0f, rotaryStartAngle, angle, true);
        g.setColour(accentGlowColour);
        g.strokePath(glowArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }
    
    // Draw background arc (track)
    {
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                                     0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(trackColour);
        g.strokePath(backgroundArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
    }
    
    // Draw value arc with gradient
    if (sliderPos > 0.0f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                                0.0f, rotaryStartAngle, angle, true);
        
        // Create gradient for value arc
        juce::ColourGradient gradient(accentColour, centreX, bounds.getY(),
                                       accentLightColour, centreX, bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }
    
    // Draw center circle with subtle gradient
    float innerRadius = radius * 0.52f;
    
    // Inner shadow effect
    juce::ColourGradient innerGradient(
        juce::Colour(0xff252540), centreX, centreY - innerRadius * 0.5f,
        juce::Colour(0xff151520), centreX, centreY + innerRadius,
        false);
    g.setGradientFill(innerGradient);
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f);
    
    // Subtle ring around center
    g.setColour(juce::Colour(0x30ffffff));
    g.drawEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f, 0.5f);
    
    // Draw pointer
    juce::Path pointer;
    float pointerLength = radius * 0.42f;
    float pointerThickness = 2.5f;
    
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -innerRadius + 5.0f,
                                 pointerThickness, pointerLength, 1.5f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    g.setColour(textColour);
    g.fillPath(pointer);
    
    // Draw center dot with glow
    float dotRadius = 3.0f;
    
    // Glow
    g.setColour(accentGlowColour);
    g.fillEllipse(centreX - dotRadius * 2, centreY - dotRadius * 2, 
                  dotRadius * 4.0f, dotRadius * 4.0f);
    
    // Center dot
    g.setColour(accentColour);
    g.fillEllipse(centreX - dotRadius, centreY - dotRadius, 
                  dotRadius * 2.0f, dotRadius * 2.0f);
}

//==============================================================================
void FIDILookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));
    
    if (!label.isBeingEdited())
    {
        auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
        
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());
        
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)(textArea.getHeight() / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

//==============================================================================
juce::Label* FIDILookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = new juce::Label();
    
    label->setJustificationType(juce::Justification::centred);
    label->setKeyboardType(juce::TextInputTarget::decimalKeyboard);
    
    label->setColour(juce::Label::textColourId, textDimColour);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    label->setColour(juce::TextEditor::textColourId, textColour);
    label->setColour(juce::TextEditor::backgroundColourId, panelColour);
    label->setColour(juce::TextEditor::highlightColourId, accentColour);
    label->setColour(juce::TextEditor::highlightedTextColourId, textColour);
    label->setColour(juce::CaretComponent::caretColourId, accentColour);
    
    label->setEditable(slider.isTextBoxEditable(), false, false);
    
    return label;
}
