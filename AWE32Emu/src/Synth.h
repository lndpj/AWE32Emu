#pragma once
#include <cstdint>
#include <array>

// DOCASNY zvukovy engine.
//
// Toto NENI emulace EMU8000 - je to jednoduchy sinusovy 32-hlasy syntetizator
// se zakladni ADSR obalkou, ktery slouzi jako funkcni zaslepka, aby bylo mozne
// hned od zacatku prehravat .mid/.xmi soubory a slyset spravny rytmus/melodii.
//
// Realna nahrada patri do projektoveho TODO seznamu, sekce 4 "Jadro emulace
// EMU8000 (register-level)":
//   - voice engine podle registrove mapy EMU8000 (misto teto zjednodusene tridy)
//   - sample playback ze SoundFont/SBK dat (viz SoundFontSbk.h) misto sinusu
//   - envelope generatory s presnymi casovymi konstantami cipu
//   - LFO1/LFO2, low-pass filtr s rezonanci
//   - chorus/reverb efektovy blok
//
// API teto tridy je navrzeno tak, aby se dalo nahradit realnou implementaci
// beze zmeny Sequenceru/main.cpp - vsechny volajici mista pouzivaji jen
// NoteOn/NoteOff/ProgramChange/ControlChange/PitchBend/RenderBlock.
class Synth
{
public:
    explicit Synth(uint32_t sampleRate);

    void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void NoteOff(uint8_t channel, uint8_t note);
    void ProgramChange(uint8_t channel, uint8_t program);     // TODO: mapovani na SoundFont patch
    void ControlChange(uint8_t channel, uint8_t controller, uint8_t value); // TODO: napr. sustain, volume, pan
    void PitchBend(uint8_t channel, int16_t value);            // TODO: aplikovat na frekvenci hlasu

    // Vyrenderuje numFrames stereo snimku (interleaved L/R int16) do out.
    void RenderBlock(int16_t* out, uint32_t numFrames);

    static constexpr int kMaxVoices = 32; // EMU8000 ma 32 hardwarovych hlasu

private:
    enum class EnvStage { Idle, Attack, Sustain, Release };

    struct Voice
    {
        bool active = false;
        uint8_t channel = 0;
        uint8_t note = 0;
        double phase = 0.0;
        double freqHz = 0.0;
        double velocityGain = 0.0;
        EnvStage stage = EnvStage::Idle;
        double envLevel = 0.0;
    };

    int FindFreeVoiceOrSteal();
    double NoteToFreqHz(uint8_t note) const;

    uint32_t m_sampleRate;
    std::array<Voice, kMaxVoices> m_voices;

    // Jednoduche pevne casove konstanty obalky (v sekundach) - placeholder,
    // realna implementace bude cist tyto hodnoty z EMU8000 envelope registru.
    static constexpr double kAttackSeconds = 0.005;
    static constexpr double kReleaseSeconds = 0.08;
};
