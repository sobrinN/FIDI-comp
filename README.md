# FIDI Comp

<div align="center">

![FIDI Comp](https://img.shields.io/badge/VST3-Compressor-00d4ff?style=for-the-badge)
![JUCE](https://img.shields.io/badge/JUCE-8.x-8dc63f?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-lightgrey?style=for-the-badge)

**A professional dynamics compressor plugin built with JUCE**

</div>

---

## Overview

FIDI Comp is a professional-grade VST3/AU dynamics compressor featuring stereo-linked detection, soft-knee compression, and parallel mixing capabilities. Built with the JUCE 8.x framework, it delivers clean, transparent compression with a modern dark interface.

## Features

- **Stereo-Linked Detection** - Maintains stereo image coherence using max of L/R channels
- **Soft Knee Compression** - Quadratic interpolation for smooth, musical transitions
- **Parallel Compression** - Built-in dry/wet mix control for New York-style compression
- **Batched Parameter Smoothing** - Zero zipper noise with optimized CPU usage
- **16-Segment GR Meter** - Real-time LED-style gain reduction visualization
- **Modern Dark UI** - Cyan accent theme with glow effects and gradient arcs

## Parameters

| Parameter     | Range           | Default | Description                    |
|---------------|-----------------|---------|--------------------------------|
| **Threshold** | -60 to 0 dB     | -20 dB  | Compression onset level        |
| **Ratio**     | 1:1 to 20:1     | 4:1     | Compression intensity          |
| **Attack**    | 0.1 to 300 ms   | 10 ms   | Time to reach full compression |
| **Release**   | 10 to 3000 ms   | 100 ms  | Time to return to unity gain   |
| **Knee**      | 0 to 20 dB      | 6 dB    | Soft knee width                |
| **Makeup**    | -12 to 24 dB    | 0 dB    | Output gain compensation       |
| **Mix**       | 0 to 100%       | 100%    | Parallel compression blend     |

## Building

### Prerequisites

- **CMake** 3.22 or higher
- **Xcode** (macOS) or **Visual Studio 2022** (Windows)
- **JUCE** 8.x (cloned automatically or via submodule)

### Quick Start

```bash
# Clone the repository with JUCE submodule
git clone --recursive https://github.com/sobrinn/FIDI-comp.git
cd FIDI-comp/FIDIComp

# Configure and build
cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release -j8
```

### Manual JUCE Setup

If not using submodules:

```bash
cd FIDI-comp/FIDIComp
git clone https://github.com/juce-framework/JUCE.git
cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release -j8
```

### Output Locations

| Format       | Location                                                   |
|--------------|------------------------------------------------------------|
| **VST3**     | `~/Library/Audio/Plug-Ins/VST3/FIDI Comp.vst3`            |
| **AU**       | `~/Library/Audio/Plug-Ins/Components/FIDI Comp.component` |
| **Standalone** | `cmake-build/FIDIComp_artefacts/Release/Standalone/`     |

## Project Structure

```
FIDIComp/
├── CMakeLists.txt              # CMake build configuration
├── FIDIComp.jucer              # Projucer project (alternative build)
├── JUCE/                       # JUCE framework
└── Source/
    ├── PluginProcessor.cpp/h   # Audio routing and state management
    ├── PluginEditor.cpp/h      # GUI layout (700x340)
    ├── Compressor.cpp/h        # DSP: envelope follower and gain
    ├── Parameters.cpp/h        # Sample-rate aware coefficient calculation
    ├── Meter.cpp/h             # Gain reduction visualization
    └── LookAndFeel.cpp/h       # Custom knob styling
```

## Technical Details

### DSP Architecture

```
Input -> Stereo Link -> Envelope Follower -> Soft Knee Gain -> Mix -> Makeup -> Output
              |                                    |
         max(|L|,|R|)                       Smoothed GR -> Meter
```

### Key Design Decisions

- **One-pole envelope follower** with adaptive attack/release coefficient selection
- **Quadratic soft knee** interpolation for C1 continuity at knee boundaries
- **Batched parameter smoothing** every 32 samples for CPU efficiency
- **Atomic floats** for lock-free metering between audio and GUI threads
- **noexcept and nodiscard** annotations for performance and safety

### Supported Sample Rates

The plugin automatically recalculates all timing coefficients for any sample rate:

- 44.1 kHz
- 48 kHz
- 96 kHz
- Any other rate

### Thread Safety

- Parameters use atomic reads from APVTS
- Meter communication via std::atomic with compare-exchange updates
- No shared mutable state between audio and GUI threads

## Development

### Alternative Build with Projucer

1. Open `FIDIComp.jucer` in the Projucer application
2. Export to your preferred IDE (Xcode or Visual Studio)
3. Build the generated project

### Adding New Parameters

1. Define parameter in `PluginProcessor::createParameterLayout()`
2. Add atomic reference in `Parameters.h`
3. Update smoothing logic in `Compressor.cpp` if needed
4. Add UI controls in `PluginEditor.cpp`

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [JUCE Framework](https://juce.com/) - Cross-platform audio plugin framework
- Inspired by classic analog compressor designs

---

<div align="center">

**Made by FIDI**

[fidi.audio](https://fidi.audio)

</div>
