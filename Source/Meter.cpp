#include "Meter.h"

//==============================================================================
Meter::Meter(std::atomic<float>& gainReduction)
    : gainReductionAtomic(gainReduction)
{
    startTimerHz(30);  // 30 FPS refresh rate
}

Meter::~Meter()
{
    stopTimer();
}

//==============================================================================
void Meter::timerCallback()
{
    // Read and reset atomic value
    float gainReduction = gainReductionAtomic.exchange(1.0f);
    
    // Convert to dB of reduction (1.0 = 0dB, 0.5 = 6dB, etc.)
    float reductionDb = -juce::Decibels::gainToDecibels(gainReduction);
    
    // Clamp to display range
    reductionDb = juce::jlimit(0.0f, maxDb, reductionDb);
    
    // Apply ballistics (smooth the display)
    float coeff = (reductionDb > displayValue) ? attackCoeff : releaseCoeff;
    displayValue += coeff * (reductionDb - displayValue);
    
    repaint();
}

//==============================================================================
void Meter::resized()
{
}

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    // Background with subtle gradient
    juce::ColourGradient bgGradient(
        juce::Colour(0xff0a0a14), bounds.getX(), bounds.getY(),
        juce::Colour(0xff0f0f1e), bounds.getX(), bounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Calculate segment dimensions
    float segmentGap = 2.0f;
    float totalGapHeight = segmentGap * (numSegments - 1);
    float segmentHeight = (bounds.getHeight() - totalGapHeight) / numSegments;
    float segmentWidth = bounds.getWidth() - 6.0f;
    float segmentX = bounds.getX() + 3.0f;
    
    // Calculate how many segments to light up
    float normalizedValue = displayValue / maxDb;
    float litSegments = normalizedValue * numSegments;
    
    for (int i = 0; i < numSegments; ++i)
    {
        float segmentY = bounds.getY() + i * (segmentHeight + segmentGap);
        
        // Determine segment color based on position with smooth gradient
        juce::Colour segmentColour;
        float position = static_cast<float>(i) / numSegments;
        
        if (position < 0.4f)
        {
            // Cyan (low GR)
            segmentColour = juce::Colour(0xff00d4ff);
        }
        else if (position < 0.65f)
        {
            // Blend cyan to yellow
            float blend = (position - 0.4f) / 0.25f;
            segmentColour = juce::Colour(0xff00d4ff).interpolatedWith(
                juce::Colour(0xffffcc00), blend);
        }
        else if (position < 0.8f)
        {
            // Blend yellow to orange
            float blend = (position - 0.65f) / 0.15f;
            segmentColour = juce::Colour(0xffffcc00).interpolatedWith(
                juce::Colour(0xffff8800), blend);
        }
        else
        {
            // Red (high GR)
            float blend = (position - 0.8f) / 0.2f;
            segmentColour = juce::Colour(0xffff8800).interpolatedWith(
                juce::Colour(0xffff4444), blend);
        }
        
        // Determine if this segment is lit
        float segmentBrightness = 0.0f;
        
        if (i < static_cast<int>(litSegments))
        {
            // Fully lit
            segmentBrightness = 1.0f;
        }
        else if (i < static_cast<int>(litSegments) + 1 && litSegments > 0)
        {
            // Partially lit (fractional segment)
            segmentBrightness = litSegments - std::floor(litSegments);
        }
        
        if (segmentBrightness > 0.01f)
        {
            // Draw glow effect for lit segments
            g.setColour(segmentColour.withAlpha(segmentBrightness * 0.3f));
            g.fillRoundedRectangle(segmentX - 1.0f, segmentY - 0.5f, 
                                   segmentWidth + 2.0f, segmentHeight + 1.0f, 2.5f);
            
            // Draw main segment
            g.setColour(segmentColour.withAlpha(segmentBrightness));
            g.fillRoundedRectangle(segmentX, segmentY, segmentWidth, segmentHeight, 2.0f);
        }
        else
        {
            // Dim unlit segments
            g.setColour(juce::Colour(0xff1a1a2e));
            g.fillRoundedRectangle(segmentX, segmentY, segmentWidth, segmentHeight, 2.0f);
        }
    }
    
    // Draw border with subtle highlight
    g.setColour(juce::Colour(0xff333344));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    
    // Top edge highlight
    g.setColour(juce::Colour(0x20ffffff));
    g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1), 
                         bounds.getX() + 4, bounds.getRight() - 4);
}
