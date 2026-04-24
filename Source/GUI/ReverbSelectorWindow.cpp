#include "ReverbSelectorWindow.h"
#include "MagnificatorLookAndFeel.h"
#include "../PluginProcessor.h"

std::unique_ptr<ReverbSelectorWindow> ReverbSelectorWindow::instance;

// =============== ReverbSelectorContent ===============

ReverbSelectorContent::ReverbSelectorContent (MagnificatorAudioProcessor& proc)
    : processor (proc)
{
    header.setText ("Reverb Type", juce::dontSendNotification);
    header.setFont (juce::Font (20.0f, juce::Font::bold));
    header.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (header);

    subHeader.setText ("Tail-only algorithm", juce::dontSendNotification);
    subHeader.setFont (juce::Font (12.0f));
    subHeader.setColour (juce::Label::textColourId,
                          MagnificatorLookAndFeel::Colours::textMuted());
    subHeader.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (subHeader);

    auto setupButton = [this] (juce::TextButton& b, int choiceIdx)
    {
        b.setClickingTogglesState (true);
        b.setRadioGroupId (0); // керуємо вручну, щоб писати у APVTS
        addAndMakeVisible (b);

        b.onClick = [this, choiceIdx]
        {
            if (auto* p = processor.apvts.getParameter (
                    MagnificatorAudioProcessor::kParamReverbType))
            {
                p->beginChangeGesture();
                // AudioParameterChoice: normalised = index / (numChoices-1)
                auto* cp = dynamic_cast<juce::AudioParameterChoice*> (p);
                if (cp)
                {
                    const float norm = (float) choiceIdx
                                     / (float) juce::jmax (1, cp->choices.size() - 1);
                    p->setValueNotifyingHost (norm);
                }
                p->endChangeGesture();
            }
            updateToggles();
        };
    };

    setupButton (room,    0);
    setupButton (hall,    1);
    setupButton (chamber, 2);
    setupButton (spring,  3);

    updateToggles();
}

void ReverbSelectorContent::updateToggles()
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (
            processor.apvts.getParameter (MagnificatorAudioProcessor::kParamReverbType)))
    {
        const int idx = p->getIndex();
        room.setToggleState    (idx == 0, juce::dontSendNotification);
        hall.setToggleState    (idx == 1, juce::dontSendNotification);
        chamber.setToggleState (idx == 2, juce::dontSendNotification);
        spring.setToggleState  (idx == 3, juce::dontSendNotification);
    }
}

void ReverbSelectorContent::paint (juce::Graphics& g)
{
    g.fillAll (MagnificatorLookAndFeel::Colours::background());
}

void ReverbSelectorContent::resized()
{
    auto area = getLocalBounds().reduced (18);

    header   .setBounds (area.removeFromTop (28));
    subHeader.setBounds (area.removeFromTop (18));
    area.removeFromTop (14);

    // 2 × 2 сітка
    auto row1 = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
    auto row2 = area.reduced (0, 4);

    const int gap = 10;
    room   .setBounds (row1.removeFromLeft ((row1.getWidth() - gap) / 2));
    row1.removeFromLeft (gap);
    hall   .setBounds (row1);

    chamber.setBounds (row2.removeFromLeft ((row2.getWidth() - gap) / 2));
    row2.removeFromLeft (gap);
    spring .setBounds (row2);
}

// =============== ReverbSelectorWindow ===============

ReverbSelectorWindow::ReverbSelectorWindow (MagnificatorAudioProcessor& proc)
    : DocumentWindow ("Magnificator — Reverb Type",
                       MagnificatorLookAndFeel::Colours::background(),
                       DocumentWindow::closeButton)
{
    setUsingNativeTitleBar (true);
    setResizable (false, false);

    auto* content = new ReverbSelectorContent (proc);
    content->setSize (320, 220);
    setContentOwned (content, true);

    centreWithSize (getWidth(), getHeight());
    setVisible (true);
    setAlwaysOnTop (true);
}

void ReverbSelectorWindow::closeButtonPressed()
{
    instance.reset();
}

ReverbSelectorWindow* ReverbSelectorWindow::showOrToggle (MagnificatorAudioProcessor& proc)
{
    if (instance)
    {
        instance.reset();
        return nullptr;
    }
    instance = std::make_unique<ReverbSelectorWindow> (proc);
    return instance.get();
}
