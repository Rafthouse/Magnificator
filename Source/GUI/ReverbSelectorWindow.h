#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

class MagnificatorAudioProcessor;

/**
    Окреме спливаюче вікно для вибору алгоритму реверба:
    Room / Hall / Chamber / Spring.
    Відкривається з головного вікна і прив'язане до параметра "revtype" APVTS.
*/
class ReverbSelectorWindow : public juce::DocumentWindow
{
public:
    explicit ReverbSelectorWindow (MagnificatorAudioProcessor& proc);
    void closeButtonPressed() override;

    static ReverbSelectorWindow* showOrToggle (MagnificatorAudioProcessor& proc);

private:
    static std::unique_ptr<ReverbSelectorWindow> instance;
};

class ReverbSelectorContent : public juce::Component
{
public:
    explicit ReverbSelectorContent (MagnificatorAudioProcessor& proc);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    MagnificatorAudioProcessor& processor;
    juce::Label          header;
    juce::Label          subHeader;
    juce::TextButton     room   { "Room" };
    juce::TextButton     hall   { "Hall" };
    juce::TextButton     chamber{ "Chamber" };
    juce::TextButton     spring { "Spring" };

    void updateToggles();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbSelectorContent)
};
