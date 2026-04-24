#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/MagnificatorLookAndFeel.h"
#include "GUI/BigKnob.h"

class MagnificatorAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit MagnificatorAudioProcessorEditor (MagnificatorAudioProcessor&);
    ~MagnificatorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Застосувати розмір UI (0 = S, 1 = M, 2 = L). */
    void applySize (int sizeIndex, bool storeInState = true);

private:
    void timerCallback() override;
    void refreshReverbNameLabel();

    MagnificatorAudioProcessor& processor;
    MagnificatorLookAndFeel     lnf;

    BigKnob          magnifyKnob;
    juce::Label      magnifyLabel;

    juce::Slider     dryWetSlider;
    juce::Label      dryWetLabel;

    juce::TextButton reverbTypeBtn  { "Reverb Type" };
    juce::TextButton sizeBtn        { "M" };

    juce::Label      titleLabel;
    juce::Label      reverbNameLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ParamAttachment  = juce::AudioProcessorValueTreeState::ParameterAttachment;

    std::unique_ptr<SliderAttachment> dryWetAttach;
    std::unique_ptr<ParamAttachment>  magnifyAttach;

    int currentSizeIndex  { 1 };
    int lastReverbTypeIdx { -1 };

    static constexpr int kBaseWidth  = 440;
    static constexpr int kBaseHeight = 380;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MagnificatorAudioProcessorEditor)
};
