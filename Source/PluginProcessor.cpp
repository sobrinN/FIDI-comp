#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Identifier FIDICompProcessor::stateIdentifier{"FIDICompState"};

//==============================================================================
FIDICompProcessor::FIDICompProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, stateIdentifier, createParameterLayout()),
      parameters(apvts),
      compressor(parameters)
{
}

FIDICompProcessor::~FIDICompProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FIDICompProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Threshold: -60 to 0 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"threshold", 1},
        "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Ratio: 1:1 to 20:1 (with skew for more resolution at lower ratios)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ratio", 1},
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    // Attack: 0.1 to 300 ms (with skew for more resolution at faster times)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"attack", 1},
        "Attack",
        juce::NormalisableRange<float>(0.1f, 300.0f, 0.1f, 0.4f),
        10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Release: 10 to 3000 ms (with skew for more resolution at faster times)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"release", 1},
        "Release",
        juce::NormalisableRange<float>(10.0f, 3000.0f, 1.0f, 0.4f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Knee: 0 to 20 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"knee", 1},
        "Knee",
        juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f),
        6.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Makeup: -12 to 24 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"makeup", 1},
        "Makeup",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Mix: 0 to 100%
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String FIDICompProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FIDICompProcessor::acceptsMidi() const
{
    return false;
}

bool FIDICompProcessor::producesMidi() const
{
    return false;
}

bool FIDICompProcessor::isMidiEffect() const
{
    return false;
}

double FIDICompProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FIDICompProcessor::getNumPrograms()
{
    return 1;
}

int FIDICompProcessor::getCurrentProgram()
{
    return 0;
}

void FIDICompProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String FIDICompProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void FIDICompProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void FIDICompProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    
    parameters.setSampleRate(sampleRate);
    compressor.reset();
    gainReductionAtomic.store(1.0f);
}

void FIDICompProcessor::releaseResources()
{
}

bool FIDICompProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto mainInput = layouts.getMainInputChannelSet();
    auto mainOutput = layouts.getMainOutputChannelSet();

    // Input and output must match
    if (mainInput != mainOutput)
        return false;

    // Support mono or stereo
    if (mainInput == juce::AudioChannelSet::mono())
        return true;
    if (mainInput == juce::AudioChannelSet::stereo())
        return true;

    return false;
}

void FIDICompProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Clear any output channels beyond input
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, numSamples);

    // Update parameters from APVTS
    parameters.update();

    // Track minimum gain reduction for metering
    float minGainReduction = 1.0f;

    if (numChannels == 0 || numSamples == 0)
        return;

    // Get write pointers
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Read input samples
        float inputL = leftChannel[sample];
        float inputR = rightChannel != nullptr ? rightChannel[sample] : inputL;

        // Store dry signal for parallel compression
        float dryL = inputL;
        float dryR = inputR;

        // Stereo-linked detection: use maximum of both channels
        float linkedLevel = std::max(std::abs(inputL), std::abs(inputR));

        // Compute gain reduction (both channels get same GR for stereo linking)
        // This also updates smoothed mix and makeup values
        float gainReduction = compressor.computeGainReduction(linkedLevel);

        // Get per-sample smoothed values (prevents zipper noise)
        float mixAmount = compressor.getSmoothedMix();
        float dryAmount = 1.0f - mixAmount;
        float makeupGain = compressor.getSmoothedMakeup();

        // Track minimum for metering
        if (gainReduction < minGainReduction)
            minGainReduction = gainReduction;

        // Apply gain reduction (wet signal)
        float wetL = inputL * gainReduction;
        float wetR = inputR * gainReduction;

        // Parallel mix
        float outputL = dryL * dryAmount + wetL * mixAmount;
        float outputR = dryR * dryAmount + wetR * mixAmount;

        // Apply makeup gain
        outputL *= makeupGain;
        outputR *= makeupGain;

        // Safety: prevent NaN/Inf in output (can occur with extreme settings)
        if (std::isnan(outputL) || std::isinf(outputL))
            outputL = 0.0f;
        if (std::isnan(outputR) || std::isinf(outputR))
            outputR = 0.0f;

        // Write output
        leftChannel[sample] = outputL;
        if (rightChannel != nullptr)
            rightChannel[sample] = outputR;
    }

    // Update atomic for metering (compare-exchange to keep minimum)
    float expected = gainReductionAtomic.load();
    while (minGainReduction < expected)
    {
        if (gainReductionAtomic.compare_exchange_weak(expected, minGainReduction))
            break;
    }
}

//==============================================================================
bool FIDICompProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FIDICompProcessor::createEditor()
{
    return new FIDICompEditor(*this);
}

//==============================================================================
void FIDICompProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void FIDICompProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FIDICompProcessor();
}
