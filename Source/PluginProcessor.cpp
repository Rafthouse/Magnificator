#include "PluginProcessor.h"
#include "PluginEditor.h"

// ---------------------------------------------------------------------------
MagnificatorAudioProcessor::MagnificatorAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    magnifyParam    = apvts.getRawParameterValue (kParamMagnify);
    dryWetParam     = apvts.getRawParameterValue (kParamDryWet);
    reverbTypeParam = apvts.getRawParameterValue (kParamReverbType);
}

juce::AudioProcessorValueTreeState::ParameterLayout
MagnificatorAudioProcessor::createParameterLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { kParamMagnify, 1 },
        "Magnify",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.0f,
        AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction ([] (float v, int) {
                return juce::String (int (v * 100.0f)) + " %"; })));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { kParamDryWet, 1 },
        "Dry / Wet",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { kParamReverbType, 1 },
        "Reverb Type",
        StringArray { "Room", "Hall", "Chamber", "Spring" },
        1));

    // UI size — не впливає на DSP, але зберігається у стан
    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { kParamUiSize, 1 },
        "UI Size",
        StringArray { "Small", "Medium", "Large" },
        1));

    return { params.begin(), params.end() };
}

// ---------------------------------------------------------------------------
void MagnificatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        (juce::uint32) samplesPerBlock,
        (juce::uint32) getTotalNumOutputChannels()
    };

    for (auto& fc : filterChains) fc.prepare (spec);
    reverbEngine.prepare (spec);

    wetBuffer.setSize (2, samplesPerBlock, false, true, true);
}

void MagnificatorAudioProcessor::releaseResources() {}

bool MagnificatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::stereo()
     && mainOut != juce::AudioChannelSet::mono())
        return false;

    return mainOut == mainIn;
}

// ---------------------------------------------------------------------------
void MagnificatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    if (numSamples == 0 || numCh == 0) return;

    // --- Оновлення параметрів (недорого — атомарні зчитування) ---
    const float magnify = magnifyParam->load();
    const float dryWet  = dryWetParam->load();

    const auto typeIdx = (int) reverbTypeParam->load();
    const auto newType = static_cast<ReverbEngine::Type> (typeIdx);
    if (newType != lastReverbType)
    {
        lastReverbType = newType;
        reverbEngine.setType (newType);
    }

    for (auto& fc : filterChains) fc.setDepth (magnify);

    // --- Готуємо wet-буфер потрібного розміру ---
    if (wetBuffer.getNumSamples() < numSamples)
        wetBuffer.setSize (2, numSamples, false, false, true);

    wetBuffer.clear();

    // --- 1) Еквалайзер (in-place на основному буфері) ---
    // Поки не підмішали реверб — основний буфер містить сигнал ПІСЛЯ EQ.
    for (int ch = 0; ch < juce::jmin (numCh, 2); ++ch)
        filterChains[(size_t) ch].processMono (buffer.getWritePointer (ch), numSamples);

    // --- 2) Реверб: беремо вхід для реверба з уже відфільтрованого сигналу ---
    // Для моно — дублюємо R = L.
    const float* inL = buffer.getReadPointer (0);
    const float* inR = (numCh > 1) ? buffer.getReadPointer (1) : inL;
    float*       wL  = wetBuffer.getWritePointer (0);
    float*       wR  = wetBuffer.getWritePointer (1);

    reverbEngine.processStereo (inL, inR, wL, wR, numSamples);

    // --- 3) Суміш: dry (= EQ-ed сигнал) + wet × (dryWet × magnify) ---
    // Головна ручка множить реверб: знизу знизу (magnify=0) — реверба 0,
    // а повзунок dryWet ставить стелю (глобальний "мокрий" рівень).
    const float wetGain = dryWet * magnify;   // лінійна крива; можна замінити на sqrt() для рівногучного mix
    const float dryGain = 1.0f;               // сухий завжди on — звичне поводження "send-insert"

    for (int ch = 0; ch < juce::jmin (numCh, 2); ++ch)
    {
        float* out = buffer.getWritePointer (ch);
        const float* wet = wetBuffer.getReadPointer (ch);

        juce::FloatVectorOperations::multiply (out, dryGain, numSamples);
        juce::FloatVectorOperations::addWithMultiply (out, wet, wetGain, numSamples);
    }
}

// ---------------------------------------------------------------------------
juce::AudioProcessorEditor* MagnificatorAudioProcessor::createEditor()
{
    return new MagnificatorAudioProcessorEditor (*this);
}

void MagnificatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MagnificatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ---------------------------------------------------------------------------
// Фабрика плагіна
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MagnificatorAudioProcessor();
}
