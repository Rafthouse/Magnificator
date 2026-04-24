#include "PluginEditor.h"
#include "GUI/ReverbSelectorWindow.h"

namespace
{
    float getSizeMultiplier (int sizeIndex)
    {
        switch (sizeIndex)
        {
            case 0: return 0.80f; // Small
            case 2: return 1.30f; // Large
            default: return 1.0f; // Medium
        }
    }
}

MagnificatorAudioProcessorEditor::MagnificatorAudioProcessorEditor (
        MagnificatorAudioProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&lnf);

    // -------- Top bar --------
    titleLabel.setText ("MAGNIFICATOR", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId,
                           MagnificatorLookAndFeel::Colours::textMuted());
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    reverbNameLabel.setFont (juce::Font (12.0f));
    reverbNameLabel.setColour (juce::Label::textColourId,
                                MagnificatorLookAndFeel::Colours::accent());
    reverbNameLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (reverbNameLabel);

    // -------- Big knob + attachment до параметра 'magnify' --------
    addAndMakeVisible (magnifyKnob);
    magnifyLabel.setText ("MAGNIFY", juce::dontSendNotification);
    magnifyLabel.setFont (juce::Font (11.0f, juce::Font::bold));
    magnifyLabel.setJustificationType (juce::Justification::centred);
    magnifyLabel.setColour (juce::Label::textColourId,
                             MagnificatorLookAndFeel::Colours::textMuted());
    addAndMakeVisible (magnifyLabel);

    if (auto* magParam = processor.apvts.getParameter (
            MagnificatorAudioProcessor::kParamMagnify))
    {
        magnifyAttach = std::make_unique<ParamAttachment> (
            *magParam,
            [this] (float v)
            {
                magnifyKnob.setValue (v, juce::dontSendNotification);
            },
            nullptr);

        // Виштовхуємо початкове значення у knob
        magnifyAttach->sendInitialUpdate();

        // Коли користувач крутить knob, оновлюємо параметр
        magnifyKnob.onValueChange = [this] (float v)
        {
            if (magnifyAttach)
                magnifyAttach->setValueAsCompleteGesture (v);
        };
    }

    // -------- Dry/Wet slider --------
    dryWetSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    dryWetSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (dryWetSlider);

    dryWetAttach = std::make_unique<SliderAttachment> (
        processor.apvts, MagnificatorAudioProcessor::kParamDryWet, dryWetSlider);

    dryWetLabel.setText ("DRY / WET", juce::dontSendNotification);
    dryWetLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    dryWetLabel.setJustificationType (juce::Justification::centredLeft);
    dryWetLabel.setColour (juce::Label::textColourId,
                             MagnificatorLookAndFeel::Colours::textMuted());
    addAndMakeVisible (dryWetLabel);

    // -------- Кнопка вибору типу реверба --------
    addAndMakeVisible (reverbTypeBtn);
    reverbTypeBtn.onClick = [this]
    {
        ReverbSelectorWindow::showOrToggle (processor);
    };

    // -------- Кнопка розміру UI --------
    addAndMakeVisible (sizeBtn);
    sizeBtn.onClick = [this]
    {
        juce::PopupMenu m;
        m.addItem (1, "Small",  true, currentSizeIndex == 0);
        m.addItem (2, "Medium", true, currentSizeIndex == 1);
        m.addItem (3, "Large",  true, currentSizeIndex == 2);
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&sizeBtn),
            [this] (int r)
            {
                if (r >= 1 && r <= 3) applySize (r - 1, true);
            });
    };

    // -------- Початковий розмір із state --------
    if (auto* sp = dynamic_cast<juce::AudioParameterChoice*> (
            processor.apvts.getParameter (MagnificatorAudioProcessor::kParamUiSize)))
    {
        currentSizeIndex = juce::jlimit (0, 2, sp->getIndex());
    }
    applySize (currentSizeIndex, false);

    refreshReverbNameLabel();

    // Timer ~10 Гц — тільки для опитування зміни типу реверба (дуже дешево).
    startTimerHz (10);
}

MagnificatorAudioProcessorEditor::~MagnificatorAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MagnificatorAudioProcessorEditor::timerCallback()
{
    // Polling замість listener'а → простіше й безпечніше за часом життя
    if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (
            processor.apvts.getParameter (MagnificatorAudioProcessor::kParamReverbType)))
    {
        const int idx = cp->getIndex();
        if (idx != lastReverbTypeIdx)
        {
            lastReverbTypeIdx = idx;
            refreshReverbNameLabel();
        }
    }
}

void MagnificatorAudioProcessorEditor::refreshReverbNameLabel()
{
    if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (
            processor.apvts.getParameter (MagnificatorAudioProcessor::kParamReverbType)))
    {
        reverbNameLabel.setText ("· " + cp->getCurrentChoiceName(),
                                   juce::dontSendNotification);
    }
}

void MagnificatorAudioProcessorEditor::applySize (int sizeIndex, bool storeInState)
{
    currentSizeIndex = juce::jlimit (0, 2, sizeIndex);
    const float mult = getSizeMultiplier (currentSizeIndex);

    if (storeInState)
    {
        if (auto* p = processor.apvts.getParameter (
                MagnificatorAudioProcessor::kParamUiSize))
        {
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (p))
            {
                const float norm = (float) currentSizeIndex
                                 / (float) juce::jmax (1, cp->choices.size() - 1);
                p->setValueNotifyingHost (norm);
            }
        }
    }

    sizeBtn.setButtonText (currentSizeIndex == 0 ? "S"
                         : currentSizeIndex == 2 ? "L" : "M");

    setSize (juce::roundToInt (kBaseWidth  * mult),
             juce::roundToInt (kBaseHeight * mult));
}

void MagnificatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    using Col = MagnificatorLookAndFeel::Colours;

    juce::ColourGradient bg (Col::background(), 0.0f, 0.0f,
                              Col::surface(),     0.0f, (float) getHeight(),
                              false);
    g.setGradientFill (bg);
    g.fillRect (getLocalBounds());

    g.setColour (Col::outline());
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f),
                             12.0f, 1.0f);
}

void MagnificatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);

    // ---- Top bar ----
    auto top = area.removeFromTop (28);
    titleLabel.setBounds (top.removeFromLeft ((int) (top.getWidth() * 0.6f)));
    reverbNameLabel.setBounds (top);

    area.removeFromTop (6);

    // ---- Bottom buttons ----
    auto bottom = area.removeFromBottom (46);
    const int btnW = 110;
    reverbTypeBtn.setBounds (bottom.removeFromLeft (btnW).reduced (2));
    bottom.removeFromLeft (10);
    sizeBtn.setBounds (bottom.removeFromRight (44).reduced (2));

    // ---- Dry/Wet ----
    auto dwArea = area.removeFromBottom (46);
    dryWetLabel.setBounds (dwArea.removeFromTop (16));
    dryWetSlider.setBounds (dwArea.reduced (0, 2));

    area.removeFromBottom (8);

    // ---- Knob ----
    const int side = juce::jmin (area.getWidth(), area.getHeight()) - 22;
    auto knobBounds = juce::Rectangle<int> (0, 0, side, side)
                        .withCentre (area.getCentre());
    magnifyKnob.setBounds (knobBounds);

    auto labelBounds = juce::Rectangle<int> (knobBounds.getX(),
                                              knobBounds.getBottom() + 4,
                                              knobBounds.getWidth(), 16);
    magnifyLabel.setBounds (labelBounds);
}
