#pragma once
#include <cstdint>
#include <vector>
#include <string>

// Sjednocena reprezentace MIDI udalosti, spolecna pro .mid (SMF) i .xmi vstup.
// XmiFile a MidiFile obe produkuji std::vector<MidiEvent> serazeny podle absoluteTick,
// ktery pak konzumuje Sequencer.

enum class MidiEventType : uint8_t
{
    NoteOff,
    NoteOn,
    PolyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
    TempoChange,   // meta 0x51 (SMF) / odvozeno z XMI, hodnota v microsekundach na ctvrtovou notu
    EndOfTrack
};

struct MidiEvent
{
    uint32_t absoluteTick = 0;
    MidiEventType type = MidiEventType::NoteOn;
    uint8_t channel = 0;   // 0-15, nevyuzito u TempoChange/EndOfTrack
    uint8_t data1 = 0;     // note / controller / program
    uint8_t data2 = 0;     // velocity / hodnota controlleru
    uint32_t tempoUsPerQuarter = 500000; // platne jen pro TempoChange
};

// Vysledek parsovani vstupniho souboru (spolecny pro MidiFile i XmiFile),
// ktery Sequencer prehrava.
struct ParsedSequence
{
    std::vector<MidiEvent> events;   // serazeno podle absoluteTick
    uint16_t ticksPerQuarterNote = 480;
    bool valid = false;
    std::string errorMessage;
};
