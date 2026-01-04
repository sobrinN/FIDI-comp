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
    
    // Background
    g.setColour(juce::Colour(0xff0a0a14));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Calculate segment dimensions
    float segmentGap = 2.0f;
    float totalGapHeight = segmentGap * (numSegments - 1);
    float segmentHeight = (bounds.getHeight() - totalGapHeight) / numSegments;
    float segmentWidth = bounds.getWidth() - 4.0f;
    float segmentX = bounds.getX() + 2.0f;
    
    // Calculate how many segments to light up
    float normalizedValue = displayValue / maxDb;
    float litSegments = normalizedValue * numSegments;
    
    for (int i = 0; i < numSegments; ++i)
    {
        float segmentY = bounds.getY() + i * (segmentHeight + segmentGap);
        
        // Determine segment color based on position
        // Cyan at bottom, yellow in middle, red at top
        juce::Colour segmentColour;
        float position = static_cast<float>(i) / numSegments;
        
        if (position < 0.5f)
        {
            // Cyan
            segmentColour = juce::Colour(0xff00d4ff);
        }
        else if (position < 0.75f)
        {
            // Yellow/orange
            segmentColour = juce::Colour(0xffffaa00);
        }
        else
        {
            // Red
            segmentColour = juce::Colour(0xffff4444);
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
            g.setColour(segmentColour.withAlpha(segmentBrightness));
        }
        else
        {
            // Dim unlit segments
            g.setColour(juce::Colour(0xff1a1a2e));
        }
        
        g.fillRoundedRectangle(segmentX, segmentY, segmentWidth, segmentHeight, 2.0f);
    }
    
    // Draw border
    g.setColour(juce::Colour(0xff333344));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}
