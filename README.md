# AWE32Emu

Základ projektu reálné emulace zvukového čipu **EMU8000** (Sound Blaster AWE32),
vznikající kombinací oficiální specifikace čipu a reverzního inženýrství DOS
ovladačů (AWEUTIL, CTVDSK.SYS a další). Cílem je nakonec C++ plugin, který
půjde zabudovat do hry a přehraje `.mid`/`.xmi` hudbu přes register-accurate
emulaci EMU8000 místo generického softsynthu.

**Tohle je zakladní kostra, ne hotová emulace.** Aktuální stav a co (ne)funguje
je popsané níže v sekci [Stav projektu](#stav-projektu).

## Co to umí teď

- Konzolová aplikace pro Visual Studio 2022 (x64, C++17)
- Parser `.mid` (Standard MIDI File, formát 0/1)
- Parser `.xmi` (Miles Sound System / AIL formát – IFF kontejner, `EVNT` chunk,
  převod XMI note-délky na explicitní Note Off)
- Základní sekvencer, který ticky ze souboru převádí na reálný čas podle
  tempo mapy
- Volitelné načtení `.sbk` (SoundFont/E-mu banka) – zatím jen informativně,
  vypíše seznam RIFF chunků, data se při přehrávání nepoužívají
- Přehrávání přes Windows `winmm`/`waveOut` (žádné externí závislosti)
- **Zvukové jádro je zatím jen placeholder** – jednoduchý 32-hlasý sinusový
  syntetizátor se základní ADSR obálkou, aby šlo od začátku slyšet správný
  rytmus a melodii. Skutečná emulace EMU8000 (voice engine, envelopy, filtr,
  LFO, chorus/reverb) je hlavní práce, která teprve přijde.

## Sestavení

1. Otevřít `AWE32Emu.sln` ve Visual Studiu 2022
2. Zvolit konfiguraci `Release` / `x64` (nebo `Debug` / `x64`)
3. Build → výsledné `AWE32Emu.exe` bude v `bin\x64\Release\`

Žádné externí knihovny ani vcpkg balíčky nejsou potřeba – projekt používá jen
standardní C++17 a Windows SDK (`winmm.lib`, linkováno automaticky).

## Použití

```
AWE32Emu.exe skladba.mid
AWE32Emu.exe skladba.xmi
AWE32Emu.exe skladba.mid --sbk banka.sbk
```

## Struktura projektu

```
AWE32Emu.sln
AWE32Emu/
  AWE32Emu.vcxproj
  src/
    main.cpp            CLI vstupní bod, parsování argumentů, hlavní smyčka
    MidiTypes.h          Sdílená reprezentace MIDI události (MidiEvent, ParsedSequence)
    MidiFile.h/.cpp       Parser .mid (SMF)
    XmiFile.h/.cpp        Parser .xmi
    Sequencer.h/.cpp      Převod tiků na reálný čas, dispatch událostí do Synth
    Synth.h/.cpp          Zvukové jádro (PLACEHOLDER – viz výše)
    SoundFontSbk.h/.cpp    Základní RIFF loader pro .sbk/.sf2
    AudioOutputWin.h/.cpp  Realtime výstup přes WinMM waveOut
```

## Stav projektu / co chybí

Podrobný rozpracovaný seznam úkolů (parsery, sekvencer, jádro EMU8000, SoundFont
vrstva, plugin API, testování, reverse engineering DOS ovladačů v IDA) je mimo
tento repozitář, v projektovém TODO dokumentu. Nejdůležitější otevřené body
přímo v tomto kódu:

- `Synth.cpp` – nahradit sinusový oscilátor register-accurate EMU8000 jádrem
  (voice engine, envelope generátory, LFO, filtr, chorus/reverb)
- `SoundFontSbk.cpp` – navázat načtená data (`shdr`, `phdr`, `sdta`/`smpl`) na
  reálné přehrávání vzorků v `Synth`
- `MidiFile.cpp`/`XmiFile.cpp` – SysEx zprávy (GS/GM/MT-32 reset), running
  status v XMI, podpora SMPTE division u SMF
- `Sequencer.cpp` – zatím renderuje po jednom snímku (`RenderBlock(..., 1)`) kvůli
  jednoduchosti přesného časování; pro výkon půjde později optimalizovat
- Plugin API – aktuálně je vše svázané v CLI `main.cpp`; oddělení do
  znovupoužitelné knihovny/API je další krok

## Poznámka k datům (licence/legalita)

Repozitář **záměrně neobsahuje** žádná vstupní data:

- žádné `.mid`/`.xmi` soubory z konkrétních her (autorská práva)
- žádné `.sbk`/`.sf2` banky (originální Creative/E-mu banky jsou chráněné)
- žádné binárky DOS ovladačů (AWEUTIL, CTVDSK.SYS apod.) ani jejich disassembly výstupy

`.gitignore` tyto typy souborů vylučuje. Pro lokální testování si je stáhněte
sami (viz VOGONS Driver Library pro DOS ovladače) a udržujte mimo commitovanou
historii repozitáře.

## Licence

Kód v tomto repozitáři je vlastní implementace, ne odvozenina z DOS ovladačů
ani z žádného existujícího SoundFont přehrávače. Licence projektu: doplnit
podle preference (např. MIT) před zveřejněním.
