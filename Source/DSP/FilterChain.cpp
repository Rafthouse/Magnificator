#include "FilterChain.h"

void FilterChain::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    juce::dsp::ProcessSpec monoSpec { spec.sampleRate,
                                      spec.maximumBlockSize,
                                      1 };

    highShelf.prepare (monoSpec);
    bell.prepare (monoSpec);

    coeffsDirty = true;
    updateCoefficients();
}

void FilterChain::reset()
{
    highShelf.reset();
    bell.reset();
}

void FilterChain::setDepth (float depth) noexcept
{
    depth = juce::jlimit (0.0f, 1.0f, depth);
    if (std::abs (depth - currentDepth) > 1.0e-4f)
    {
        currentDepth = depth;
        coeffsDirty  = true;
    }
}

void FilterChain::updateCoefficients()
{
    if (! coeffsDirty) return;

    // --- High-shelf ---
    // Плавна крива: depth^0.7 дає відчутну дію вже на початку ходу ручки,
    // але не "вибухову" на максимумі.
    const float shelfCurve = std::pow (currentDepth, 0.7f);
    const float shelfGainDb = highShelfMaxCut * shelfCurve;
    const float shelfGain   = juce::Decibels::decibelsToGain (shelfGainDb);

    // Q ~ 0.707 → м'який shelf без резонансу
    *highShelf.coefficients = *Coeffs::makeHighShelf (sampleRate,
                                                      highShelfFreqHz,
                                                      0.707f,
                                                      shelfGain);

    // --- Bell @ 250 Hz ---
    const float bellCurve  = std::pow (currentDepth, 0.85f);
    const float bellGainDb = bellMaxCut * bellCurve;
    const float bellGain   = juce::Decibels::decibelsToGain (bellGainDb);

    *bell.coefficients = *Coeffs::makePeakFilter (sampleRate,
                                                  bellFreqHz,
                                                  bellQ,
                                                  bellGain);

    coeffsDirty = false;
}

void FilterChain::processMono (float* data, int numSamples) noexcept
{
    updateCoefficients();

    juce::dsp::AudioBlock<float>         block (&data, 1, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    highShelf.process (ctx);
    bell.process (ctx);
}
