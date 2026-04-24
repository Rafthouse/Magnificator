#include "MagnificatorLookAndFeel.h"

MagnificatorLookAndFeel::MagnificatorLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours::background());
    setColour (juce::Label::textColourId,        Colours::textPrimary());
    setColour (juce::TextButton::buttonColourId, Colours::surfaceHi());
    setColour (juce::TextButton::textColourOffId, Colours::textPrimary());
    setColour (juce::ComboBox::backgroundColourId, Colours::surfaceHi());
    setColour (juce::ComboBox::textColourId, Colours::textPrimary());
    setColour (juce::ComboBox::outlineColourId, Colours::outline());
    setColour (juce::PopupMenu::backgroundColourId, Colours::surface());
    setColour (juce::PopupMenu::textColourId, Colours::textPrimary());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accentSoft());
    setColour (juce::PopupMenu::highlightedTextColourId, Colours::textPrimary());
}

juce::Typeface::Ptr MagnificatorLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    // Використовуємо системний шрифт за замовчуванням — на всіх ОС виглядає сучасно.
    return juce::LookAndFeel_V4::getTypefaceForFont (f);
}

juce::Font MagnificatorLookAndFeel::getLabelFont (juce::Label& label)
{
    auto f = juce::LookAndFeel_V4::getLabelFont (label);
    f.setHeight (f.getHeight() * 1.0f);
    return f;
}

void MagnificatorLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                     juce::Button& b,
                                                     const juce::Colour& base,
                                                     bool hover, bool down)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (1.0f);
    const float radius = 10.0f;

    juce::Colour fill = Colours::surfaceHi();
    if (b.getToggleState()) fill = Colours::accent().withAlpha (0.22f);
    if (hover)              fill = fill.brighter (0.05f);
    if (down)               fill = fill.darker (0.1f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (b.getToggleState() ? Colours::accent() : Colours::outline());
    g.drawRoundedRectangle (bounds, radius, b.getToggleState() ? 1.5f : 1.0f);

    juce::ignoreUnused (base);
}

void MagnificatorLookAndFeel::drawButtonText (juce::Graphics& g,
                                               juce::TextButton& b,
                                               bool, bool)
{
    g.setColour (b.getToggleState() ? Colours::accent() : Colours::textPrimary());
    g.setFont (juce::Font (juce::jmin (15.0f, b.getHeight() * 0.42f),
                            juce::Font::bold));
    g.drawText (b.getButtonText(), b.getLocalBounds(),
                juce::Justification::centred, true);
}

void MagnificatorLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                                 int x, int y, int w, int h,
                                                 float sliderPos,
                                                 float, float,
                                                 juce::Slider::SliderStyle style,
                                                 juce::Slider& s)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos,
                                                 0.0f, 0.0f, style, s);
        return;
    }

    const float trackH = 4.0f;
    const float cy     = y + h * 0.5f;
    juce::Rectangle<float> track ((float) x, cy - trackH * 0.5f,
                                   (float) w, trackH);

    g.setColour (Colours::outline());
    g.fillRoundedRectangle (track, trackH * 0.5f);

    juce::Rectangle<float> fill = track.withWidth (sliderPos - (float) x);
    g.setColour (Colours::accent());
    g.fillRoundedRectangle (fill, trackH * 0.5f);

    // Thumb
    const float r = 7.0f;
    g.setColour (Colours::accent());
    g.fillEllipse (sliderPos - r, cy - r, r * 2, r * 2);
    g.setColour (Colours::background());
    g.fillEllipse (sliderPos - r * 0.5f, cy - r * 0.5f, r, r);
}
