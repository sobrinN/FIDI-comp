# FIDI Comp - VST3 Dynamics Compressor

## Overview

FIDI Comp is a professional VST3/AU compressor plugin built with JUCE framework. Features stereo-linked compression with soft-knee, parallel mix, and real-time gain reduction metering.

## Project Structure

```
FIDIComp/
├── FIDIComp.jucer          # Projucer project file
├── CMakeLists.txt          # CMake build config
├── JUCE/                   # JUCE framework (git submodule)
└── Source/
    ├── PluginProcessor.*   # AudioProcessor - routing, state, APVTS
    ├── PluginEditor.*      # GUI - knobs, meter, 700x340 layout
    ├── Compressor.*        # DSP - envelope follower, soft-knee gain
    ├── Parameters.*        # Coefficient calculation from APVTS
    ├── Meter.*             # Timer-driven GR visualization (30fps)
    └── LookAndFeel.*       # Custom rotary slider styling with glow
```

## Parameters

| ID | Range | Default | Description |
|----|-------|---------|-------------|
| `threshold` | -60 to 0 dB | -20 | Compression threshold |
| `ratio` | 1:1 to 20:1 | 4:1 | Compression ratio |
| `attack` | 0.1-300 ms | 10 | Attack time |
| `release` | 10-3000 ms | 100 | Release time |
| `knee` | 0-20 dB | 6 | Soft knee width |
| `makeup` | -12 to 24 dB | 0 | Output gain |
| `mix` | 0-100% | 100 | Dry/wet blend |

## Architecture

**DSP Flow:**
```
Input → Stereo Link (max) → Envelope Follower → Soft Knee → Mix → Makeup → Output
```

**Key Features:**
- All 7 parameters have per-sample smoothing (no zipper noise)
- Stereo-linked detection using max of L/R
- Soft knee with quadratic interpolation
- Thread-safe atomic metering

## Build

```bash
# Clone with JUCE submodule
git clone --recursive https://github.com/YOUR_USERNAME/FIDI-comp.git
cd FIDI-comp/FIDIComp

# Or if JUCE not included, clone it
git clone https://github.com/juce-framework/JUCE.git

# Build with CMake
cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release -j8
```

## Output

- **VST3**: `~/Library/Audio/Plug-Ins/VST3/FIDI Comp.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/FIDI Comp.component`

## Key Files

| File | Purpose |
|------|---------|
| `PluginProcessor.cpp` | Audio processing loop |
| `Compressor.cpp` | Envelope + gain computation |
| `Parameters.cpp` | Coefficient calculation |
| `PluginEditor.cpp` | GUI layout (700x340) |
| `LookAndFeel.cpp` | Custom knob styling |

## Dependencies

- **JUCE 8.x** (included or global)
- **CMake 3.22+**
- **Xcode** (macOS) or **Visual Studio 2022** (Windows)
