#pragma once

#include "stdint.hpp"

// ------------------------------------------------------------------------------------------------
// Compiler Settings
// ------------------------------------------------------------------------------------------------

// Comment out to disable SDL specific sound code
#define COMPILE_SOUND_CODE 1

// ------------------------------------------------------------------------------------------------
// Debug Settings
// ------------------------------------------------------------------------------------------------

const bool DEBUG_LEVEL = false;

// Force AI to play the levels
const bool FORCE_AI = false;

// ------------------------------------------------------------------------------------------------
// General useful stuff
// ------------------------------------------------------------------------------------------------

// SYSTEM_WATCHDOG is used for Linux builds, mostly targeting RPi Zero 2W in a cabinet situation
// where overclocking is needed. The watchdog will restart the machine if it hangs.
#define SYSTEM_WATCHDOG "/dev/watchdog0"

// Internal Sega OutRun Screen Properties
const uint16_t S16_WIDTH      = 320;
const uint16_t S16_HEIGHT     = 224;

// Internal Widescreen Width
// JJP - was 398. Using 404 allows for optimisation of Blargg filter with SIMD.
const uint16_t S16_WIDTH_WIDE = 404;

// Palette Address in Memory
const uint32_t S16_PALETTE_BASE    = 0x120000;

// Number of Palette Entries
const uint16_t S16_PALETTE_ENTRIES = 0x1000;

// Number of stages
const uint16_t STAGES = 15;

// Hard Coded End Point of every level
const static uint16_t ROAD_END      = 0x79C;

// End Point of level for CPU1, including horizon
const static uint16_t ROAD_END_CPU1 = 0x904;

// Default timer used for hi-score entry
const static uint8_t HIGHSCORE_TIMER = 0x30;

// Default timer used for music selection (was 15 seconds on original/old romset)
const static uint8_t MUSIC_TIMER = 0x30;

enum
{
    BIT_0 = 0x01,
    BIT_1 = 0x02,
    BIT_2 = 0x04,
    BIT_3 = 0x08,
    BIT_4 = 0x10,
    BIT_5 = 0x20,
    BIT_6 = 0x40,
    BIT_7 = 0x80,
    BIT_8 = 0x100,
    BIT_9 = 0x200,
    BIT_A = 0x400
};

// Alignment macro for 64-bit optimisation
#if defined(_MSC_VER)
#define ALIGN64 __declspec(align(64))
#elif defined(__GNUC__)
#define ALIGN64 __attribute__((aligned(64)))
#else
#define ALIGN64
#pragma message("WARNING: ALIGN64 not defined for this compiler.")
#endif
