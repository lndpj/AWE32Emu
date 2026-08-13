#pragma once
#include "MidiTypes.h"
#include <string>

// Parser Standard MIDI File (.mid), format 0 i 1.
// TODO (viz projektovy TODO seznam, sekce 1.1): SysEx eventy, format 2, edge-case
// varianty division = SMPTE misto ticks-per-quarter.
namespace MidiFile
{
    ParsedSequence Load(const std::string& path);
}
