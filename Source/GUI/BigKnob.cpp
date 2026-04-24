#include "BigKnob.h"
#include "MagnificatorLookAndFeel.h"

BigKnob::BigKnob()
{
    setOpaque (false);
    startTimerHz (30); // ≤ 30 fps — обмежені GUI-ресурси
}

BigKnob::~BigKnob() { stopTimer(); }

void BigKnob::setValue (float newNormalised, juce::NotificationType n)
{
    newNormalised = juce::jlimit (0.0f, 1.0f, newNormalised);
    if (std::abs (newNormalised - currentValue) < 1.0e-4f) return;

    currentValue = newNormalised;
    if (n != juce::dontSendNotification && onValueChange)
        onValueChange (currentValue);
}

void BigKnob::timerCallback()
{
    // М'яко наздоганяємо цільове значення для анімації glow-а.
    const float smoothing = 0.22f;
    const float prev = displayValue;
    displayValue += (currentValue - displayValue) * smoothing;

    // Repaint лише при помітній зміні -> економія CPU
    if (std::abs (displayValue - prev) > 1.0e-3f)
        repaint();
}

void BigKnob::resized() {}

void BigKnob::paint (juce::Graphics& g)
{
    using Col = MagnificatorLookAndFeel::Colours;

    auto bounds = getLocalBounds().toFloat().reduced (6.0f);
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    const float knobR    = radius * 0.82f;
    const float ringR    = radius * 0.96f;
    const float trackW   = radius * 0.09f;

    const float angle = juce::jmap (currentValue, 0.0f, 1.0f,
                                     kStartAngle, kEndAngle);

    // -------- Glow (дешево: 2 ellipse'и з alpha, залежні від значення) --------
    if (displayValue > 0.005f)
    {
        const float glowStrength = juce::jlimit (0.0f, 1.0f, displayValue);
        const float glowR = radius * (1.08f + 0.18f * glowStrength);
        juce::ColourGradient glowGrad (
            Col::accent().withAlpha (0.35f * glowStrength), cx, cy,
            Col::accent().withAlpha (0.0f), cx + glowR, cy, true);
        g.setGradientFill (glowGrad);
        g.fillEllipse (cx - glowR, cy - glowR, glowR * 2.0f, glowR * 2.0f);
    }

    // -------- Фонове кільце track -----------
    juce::Path track;
    track.addCentredArc (cx, cy, ringR, ringR, 0.0f,
                          kStartAngle, kEndAngle, true);
    g.setColour (Col::outline());
    g.strokePath (track, juce::PathStrokeType (trackW,
                            juce::PathStrokeType::curved,
                            juce::PathStrokeType::rounded));

    // -------- Активна дуга ---------
    juce::Path active;
    active.addCentredArc (cx, cy, ringR, ringR, 0.0f,
                           kStartAngle, angle, true);
    g.setColour (Col::accent());
    g.strokePath (active, juce::PathStrokeType (trackW,
                             juce::PathStrokeType::curved,
                             juce::PathStrokeType::rounded));

    // -------- Корпус knob -----------
    // Радіальний градієнт для відчуття об'єму
    juce::ColourGradient body (
        Col::surfaceHi().brighter (0.12f), cx - knobR * 0.3f, cy - knobR * 0.4f,
        Col::surface().darker (0.25f),    cx + knobR,         cy + knobR, true);
    g.setGradientFill (body);
    g.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

    // Тонкий зовнішній контур
    g.setColour (Col::outline());
    g.drawEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, 1.0f);

    // Внутрішнє "вікно" для глибини
    const float innerR = knobR * 0.72f;
    juce::ColourGradient inner (
        Col::surface().darker (0.3f),     cx, cy - innerR,
        Col::surfaceHi().brighter (0.05f), cx, cy + innerR, false);
    g.setGradientFill (inner);
    g.fillEllipse (cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f);

    // -------- Pointer (indicator line) -----------
    const float pointerLen = knobR * 0.58f;
    const float pointerW   = juce::jmax (2.0f, radius * 0.06f);

    juce::Path pointer;
    pointer.startNewSubPath (0.0f, -knobR * 0.1f);
    pointer.lineTo (0.0f, -knobR * 0.1f - pointerLen);

    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (angle - juce::MathConstants<float>::halfPi)
                        .translated (cx, cy));
    g.setColour (Col::accent().withMultipliedBrightness (1.0f + 0.4f * displayValue));
    g.strokePath (pointer, juce::PathStrokeType (pointerW,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    g.restoreState();

    // -------- Процентовий індикатор у центрі -----------
    const juce::String pct = juce::String (int (currentValue * 100.0f)) + "%";
    g.setColour (Col::textMuted());
    g.setFont (juce::Font (radius * 0.24f, juce::Font::bold));
    g.drawText (pct, getLocalBounds(), juce::Justification::centred);
}

// ---------------- Інтерактив ----------------

void BigKnob::mouseDown (const juce::MouseEvent& e)
{
    dragStartY   = e.getPosition().y;
    dragStartVal = currentValue;
}

void BigKnob::mouseDrag (const juce::MouseEvent& e)
{
    // Вертикальний drag: 180 пікселів = повний хід 0..1.
    const float dy = (float) (dragStartY - e.getPosition().y);
    const float fineMult = e.mods.isShiftDown() ? 0.25f : 1.0f;
    const float sensitivity = 1.0f / 180.0f * fineMult;
    setValue (dragStartVal + dy * sensitivity);
}

void BigKnob::mouseWheelMove (const juce::MouseEvent&,
                               const juce::MouseWheelDetails& w)
{
    const float step = 0.03f;
    setValue (currentValue + w.deltaY * step);
}

void BigKnob::mouseDoubleClick (const juce::MouseEvent&)
{
    setValue (0.0f);
}
