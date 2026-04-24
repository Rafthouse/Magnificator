#pragma once
#include <JuceHeader.h>
#include "DSP/FilterChain.h"
#include "DSP/ReverbEngine.h"

class MagnificatorAudioProcessor : public juce::AudioProcessor
{
public:
    MagnificatorAudioProcessor();
    ~MagnificatorAudioProcessor() override = default;

    // ---- AudioProcessor API ----
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Magnificator"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- Публічний API ----
    juce::AudioProcessorValueTreeState apvts;

    static constexpr const char* kParamMagnify     = "magnify";
    static constexpr const char* kParamDryWet      = "drywet";
    static constexpr const char* kParamReverbType  = "revtype";
    static constexpr const char* kParamUiSize      = "uisize";

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::array<FilterChain, 2> filterChains; // L/R
    ReverbEngine               reverbEngine;

    juce::AudioBuffer<float>   wetBuffer;

    // Кешовані атомарні покажчики на значення параметрів
    std::atomic<float>* magnifyParam    { nullptr };
    std::atomic<float>* dryWetParam     { nullptr };
    std::atomic<float>* reverbTypeParam { nullptr };

    ReverbEngine::Type lastReverbType { ReverbEngine::Type::Hall };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MagnificatorAudioProcessor)
};
