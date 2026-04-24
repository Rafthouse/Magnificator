#pragma once
#include <juce_dsp/juce_dsp.h>

/**
    Послідовний ланцюг:
      1) High-shelf filter  — зрізає верхи в міру накручення ручки
      2) Bell (peaking) filter на 250 Гц — приглушує "бруд" у низько-середньому діапазоні

    Обидва фільтри мають керовану "глибину" (depthNormalised ∈ [0..1]),
    яка відображається на відповідні dB-значення.
*/
class FilterChain
{
public:
    FilterChain() = default;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    /** @param depth 0..1 — безпосередньо зі значення головної ручки */
    void setDepth (float depth) noexcept;

    /** Процесинг одного моно-каналу, in-place */
    void processMono (float* data, int numSamples) noexcept;

private:
    void updateCoefficients();

    using IIR = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    IIR highShelf;
    IIR bell;

    double sampleRate { 44100.0 };
    float  currentDepth { 0.0f };
    bool   coeffsDirty { true };

    // Характеристики
    static constexpr float highShelfFreqHz = 4000.0f; // точка зламу shelf
    static constexpr float highShelfMaxCut = -18.0f;  // дБ на max ручки
    static constexpr float bellFreqHz      = 250.0f;
    static constexpr float bellQ           = 1.0f;
    static constexpr float bellMaxCut      = -9.0f;   // дБ на max ручки
};
