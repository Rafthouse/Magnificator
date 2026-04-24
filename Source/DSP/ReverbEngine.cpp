#include "ReverbEngine.h"

// ---------- Spring channel -----------------------------------------------

void ReverbEngine::SpringChannel::prepare (double sr)
{
    sampleRate = sr;
    const int maxDelaySamples = (int) std::ceil (sr * 0.1); // 100 мс запасу
    modDelay.setMaximumDelayInSamples (maxDelaySamples);
    modDelay.prepare ({ sr, (juce::uint32) 512, (juce::uint32) 1 });

    // Різні довжини allpass ліній — ключ до правдоподібної дисперсії.
    const float apTimesMs[kNumAllpass]     = { 4.7f, 7.3f, 11.1f, 13.8f, 17.5f, 21.9f };
    const float apFeedbackTbl[kNumAllpass] = { 0.72f, 0.70f, 0.68f, 0.66f, 0.64f, 0.62f };

    for (int i = 0; i < kNumAllpass; ++i)
    {
        const int samples = juce::jmax (4, (int) (sr * apTimesMs[i] * 0.001));
        allpass[i].setMaximumDelayInSamples (samples + 4);
        allpass[i].prepare ({ sr, (juce::uint32) 512, (juce::uint32) 1 });
        allpass[i].setDelay ((float) samples);
        apFeedback[i] = apFeedbackTbl[i];
    }

    // LFO ~ 0.7 Гц для модуляції delay-line
    phaseInc = (float) (juce::MathConstants<double>::twoPi * 0.7 / sr);

    dampingCoef = 0.35f; // внутрішня фільтрація hi-damp у feedback петлі
    reset();
}

void ReverbEngine::SpringChannel::reset()
{
    for (auto& ap : allpass) ap.reset();
    modDelay.reset();
    phase = 0.0f;
    lastOut = 0.0f;
    damperState = 0.0f;
}

float ReverbEngine::SpringChannel::process (float input) noexcept
{
    // allpass каскад — дисперсія
    float x = input;
    for (int i = 0; i < kNumAllpass; ++i)
    {
        const float delayed = allpass[i].popSample (0);
        const float v       = x + delayed * apFeedback[i];
        allpass[i].pushSample (0, v);
        x = delayed - v * apFeedback[i];
    }

    // Модульована delay-line + зворотний зв'язок з демпфером
    phase += phaseInc;
    if (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    const float mod     = std::sin (phase) * modDepthMs;
    const float delayMs = baseDelayMs + mod;
    const float delaySamples = juce::jmax (1.0f,
                                delayMs * 0.001f * (float) sampleRate);
    modDelay.setDelay (delaySamples);

    const float delayed = modDelay.popSample (0);

    // Просте 1-поле згладжування (high-damp у петлі)
    damperState = damperState + dampingCoef * (delayed - damperState);

    const float y = x + damperState * feedback;
    modDelay.pushSample (0, y);

    lastOut = delayed;
    return delayed * 0.6f; // трохи приборкаємо рівень
}

// ---------- ReverbEngine -------------------------------------------------

ReverbEngine::ReverbEngine() = default;

void ReverbEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    freeverb.prepare (spec);

    for (auto& ch : spring) ch.prepare (spec.sampleRate);

    setType (currentType);
    isPrepared = true;
}

void ReverbEngine::reset()
{
    freeverb.reset();
    for (auto& ch : spring) ch.reset();
}

void ReverbEngine::setType (Type t) noexcept
{
    currentType = t;
    setFreeverbParams (t);
}

void ReverbEngine::setFreeverbParams (Type t)
{
    // Wet = 1.0, Dry = 0.0 — сухий підмішуємо самі в ProcessBlock.
    // Width = 1.0, freezeMode = 0.0.
    switch (t)
    {
        case Type::Room:
            freeverbParams.roomSize = 0.45f;
            freeverbParams.damping  = 0.55f;
            freeverbParams.width    = 1.0f;
            break;

        case Type::Hall:
            freeverbParams.roomSize = 0.85f;
            freeverbParams.damping  = 0.30f;
            freeverbParams.width    = 1.0f;
            break;

        case Type::Chamber:
            freeverbParams.roomSize = 0.65f;
            freeverbParams.damping  = 0.45f;
            freeverbParams.width    = 0.85f;
            break;

        default: // Spring — Freeverb не задіюємо
            return;
    }

    freeverbParams.wetLevel   = 1.0f;
    freeverbParams.dryLevel   = 0.0f;
    freeverbParams.freezeMode = 0.0f;

    freeverb.setParameters (freeverbParams);
}

void ReverbEngine::processStereo (const float* inL, const float* inR,
                                  float* wetL, float* wetR,
                                  int numSamples) noexcept
{
    if (currentType == Type::Spring)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            wetL[i] = spring[0].process (inL[i]);
            wetR[i] = spring[1].process (inR[i]);
        }
        return;
    }

    // Freeverb: копіюємо вхід у wet-буфер і обробляємо in-place.
    juce::FloatVectorOperations::copy (wetL, inL, numSamples);
    juce::FloatVectorOperations::copy (wetR, inR, numSamples);

    float* channels[2] = { wetL, wetR };
    juce::dsp::AudioBlock<float> block (channels, 2, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    freeverb.process (ctx);
}
