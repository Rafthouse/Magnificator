#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

/**
    Material-inspired LookAndFeel.
    Темна палітра з акцентним кольором, плавні скруглення, м'які тіні.
*/
class MagnificatorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MagnificatorLookAndFeel();

    // --- Сучасна палітра ---
    struct Colours
    {
        static inline juce::Colour background()  { return juce::Colour (0xFF121418); } // near-black
        static inline juce::Colour surface()     { return juce::Colour (0xFF1A1D23); } // card
        static inline juce::Colour surfaceHi()   { return juce::Colour (0xFF252A32); } // elevated card
        static inline juce::Colour outline()     { return juce::Colour (0xFF2E3440); }
        static inline juce::Colour textPrimary() { return juce::Colour (0xFFE6E9EF); }
        static inline juce::Colour textMuted()   { return juce::Colour (0xFF8A93A1); }
        static inline juce::Colour accent()      { return juce::Colour (0xFF7AE0E0); } // cyan/mint
        static inline juce::Colour accentSoft()  { return juce::Colour (0x407AE0E0); }
        static inline juce::Colour danger()      { return juce::Colour (0xFFE08A7A); }
    };

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;
    juce::Font getLabelFont (juce::Label&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    // Решту використовуємо від LookAndFeel_V4 за замовчуванням —
    // наш головний knob малюється кастомно в BigKnob.
};
