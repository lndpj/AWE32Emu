#include "Synth.h"
#include <cmath>

namespace
{
    constexpr double kPi = 3.14159265358979323846;
}

Synth::Synth(uint32_t sampleRate)
    : m_sampleRate(sampleRate)
{
}

double Synth::NoteToFreqHz(uint8_t note) const
{
    return 440.0 * std::pow(2.0, (static_cast<double>(note) - 69.0) / 12.0);
}

int Synth::FindFreeVoiceOrSteal()
{
    for (int i = 0; i < kMaxVoices; ++i)
        if (!m_voices[i].active)
            return i;

    // Vsech 32 hlasu obsazeno - ukradnout ten nejtisejsi (nejblize doznele).
    // Skutecny EMU8000 ma vlastni voice-stealing prioritu (viz TODO sekce 4);
    // tohle je jen rozumna zaslepka.
    int quietest = 0;
    double quietestLevel = m_voices[0].envLevel;
    for (int i = 1; i < kMaxVoices; ++i)
    {
        if (m_voices[i].envLevel < quietestLevel)
        {
            quietest = i;
            quietestLevel = m_voices[i].envLevel;
        }
    }
    return quietest;
}

void Synth::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (velocity == 0)
    {
        NoteOff(channel, note);
        return;
    }

    int idx = FindFreeVoiceOrSteal();
    Voice& v = m_voices[idx];
    v.active = true;
    v.channel = channel;
    v.note = note;
    v.phase = 0.0;
    v.freqHz = NoteToFreqHz(note);
    v.velocityGain = static_cast<double>(velocity) / 127.0;
    v.stage = EnvStage::Attack;
    v.envLevel = 0.0;
}

void Synth::NoteOff(uint8_t channel, uint8_t note)
{
    for (auto& v : m_voices)
    {
        if (v.active && v.channel == channel && v.note == note && v.stage != EnvStage::Release)
        {
            v.stage = EnvStage::Release;
        }
    }
}

void Synth::ProgramChange(uint8_t /*channel*/, uint8_t /*program*/)
{
    // TODO: az bude napojena SoundFontSbk vrstva, tady se vybere patch/instrument
    // pro dany kanal misto ted pouziteho pevneho sinusoveho generatoru.
}

void Synth::ControlChange(uint8_t /*channel*/, uint8_t /*controller*/, uint8_t /*value*/)
{
    // TODO: sustain pedal (64), channel volume (7), pan (10), atd.
}

void Synth::PitchBend(uint8_t /*channel*/, int16_t /*value*/)
{
    // TODO: prepocitat freqHz aktivnich hlasu na danem kanalu podle pitch bend range.
}

void Synth::RenderBlock(int16_t* out, uint32_t numFrames)
{
    const double attackStep = 1.0 / (kAttackSeconds * m_sampleRate);
    const double releaseStep = 1.0 / (kReleaseSeconds * m_sampleRate);

    for (uint32_t frame = 0; frame < numFrames; ++frame)
    {
        double mix = 0.0;

        for (auto& v : m_voices)
        {
            if (!v.active)
                continue;

            switch (v.stage)
            {
            case EnvStage::Attack:
                v.envLevel += attackStep;
                if (v.envLevel >= 1.0)
                {
                    v.envLevel = 1.0;
                    v.stage = EnvStage::Sustain;
                }
                break;
            case EnvStage::Sustain:
                break;
            case EnvStage::Release:
                v.envLevel -= releaseStep;
                if (v.envLevel <= 0.0)
                {
                    v.envLevel = 0.0;
                    v.active = false;
                }
                break;
            default:
                break;
            }

            if (v.active)
            {
                double sample = std::sin(v.phase) * v.envLevel * v.velocityGain;
                mix += sample;

                v.phase += 2.0 * kPi * v.freqHz / m_sampleRate;
                if (v.phase > 2.0 * kPi)
                    v.phase -= 2.0 * kPi;
            }
        }

        // Jednoduchy soft-limiter, aby vic soucasnych hlasu neorizlo signal.
        mix *= 0.2;
        if (mix > 1.0) mix = 1.0;
        if (mix < -1.0) mix = -1.0;

        int16_t sampleI16 = static_cast<int16_t>(mix * 32767.0);
        out[frame * 2 + 0] = sampleI16; // L
        out[frame * 2 + 1] = sampleI16; // R (zatim mono zdvojene - pan viz TODO ControlChange)
    }
}
