#pragma once
#include <juce_dsp/juce_dsp.h>

/**
    Реверб-ядро "tail-only":
      • Алгоритм Freeverb (JUCE) для Room / Hall / Chamber — без predelay,
        без окремої секції ER (Freeverb за архітектурою не формує явних ER).
      • Кастомний алгоритм Spring: каскад allpass-фільтрів + модульована
        delay-line, що відтворює характерну "бойнг"-дисперсію пружини.

      Тут немає сухого сигналу на виході — повертаємо лише "хвіст".
      Сухий сигнал підмішується на рівні плагіна (у ProcessBlock).
*/
class ReverbEngine
{
public:
    enum class Type { Room = 0, Hall, Chamber, Spring };

    ReverbEngine();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void setType (Type t) noexcept;
    Type getType() const noexcept { return currentType; }

    /** Блоковий процесинг стерео. wetOut має бути вже підготовленим буфером того ж розміру. */
    void processStereo (const float* inL, const float* inR,
                        float* wetL, float* wetR,
                        int numSamples) noexcept;

private:
    // ---- Freeverb-гілка ----
    void setFreeverbParams (Type t);
    juce::dsp::Reverb freeverb;
    juce::dsp::Reverb::Parameters freeverbParams;

    // ---- Spring-гілка: allpass-дисперсія + модулятор ----
    struct SpringChannel
    {
        static constexpr int kNumAllpass = 6;

        std::array<juce::dsp::DelayLine<float,
                    juce::dsp::DelayLineInterpolationTypes::Linear>,
                   kNumAllpass> allpass;
        std::array<float, kNumAllpass> apFeedback {};

        juce::dsp::DelayLine<float,
                    juce::dsp::DelayLineInterpolationTypes::Linear> modDelay
                        { (int) (0.09 * 96000) }; // до 90 мс @ 96k

        double sampleRate = 44100.0;
        float phase       = 0.0f;
        float phaseInc    = 0.0f;
        float lastOut     = 0.0f;
        float dampingCoef = 0.5f;
        float damperState = 0.0f;
        float baseDelayMs = 45.0f;
        float modDepthMs  = 2.5f;
        float feedback    = 0.55f;

        void prepare (double sr);
        void reset();
        float process (float input) noexcept;
    };

    std::array<SpringChannel, 2> spring;

    Type     currentType { Type::Hall };
    double   sampleRate  { 44100.0 };
    bool     isPrepared  { false };
};
