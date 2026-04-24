#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

/**
    Кастомний головний Magnify knob:
      • Кругла "лінза" з ободом і внутрішнім градієнтом
      • Анімований glow, що підсилюється з ростом значення
      • Repaint лише коли значення змінилося (низьке CPU)
      • Frame rate обмежено 30 Гц -> < 25% ресурсів GUI
*/
class BigKnob : public juce::Component,
                private juce::Timer
{
public:
    BigKnob();
    ~BigKnob() override;

    // --- Slider-like API ---
    void  setValue (float newNormalised, juce::NotificationType n = juce::sendNotification);
    float getValue() const noexcept { return currentValue; }

    std::function<void (float)> onValueChange;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    float currentValue  { 0.0f };  // 0..1
    float displayValue  { 0.0f };  // згладжене для анімації glow
    float dragStartVal  { 0.0f };
    int   dragStartY    { 0 };

    // Кут повороту, рад. Мінімум ~ -225°, максимум ~ +45° від вертикалі.
    static constexpr float kStartAngle = juce::MathConstants<float>::pi * 1.25f;
    static constexpr float kEndAngle   = juce::MathConstants<float>::pi * 2.75f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigKnob)
};
