# FIDI Comp

<div align="center">

![FIDI Comp](https://img.shields.io/badge/VST3-Compressor-00d4ff?style=for-the-badge)
![JUCE](https://img.shields.io/badge/JUCE-8.x-8dc63f?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-lightgrey?style=for-the-badge)

**A professional dynamics compressor plugin built with JUCE**

</div>

---

## Features

- **Stereo-Linked Detection** - Consistent stereo image using max of L/R channels
- **Soft Knee Compression** - Quadratic interpolation for smooth transition
- **Parallel Compression** - Built-in dry/wet mix control
- **Batched Parameter Smoothing** - Zero zipper noise with efficient CPU usage
- **Real-Time GR Meter** - 16-segment LED-style visualization
- **Modern Dark UI** - Vibrant cyan accent with glow effects

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| **Threshold** | -60 to 0 dB | Compression onset level |
| **Ratio** | 1:1 to 20:1 | Compression intensity |
| **Attack** | 0.1 to 300 ms | Attack time |
| **Release** | 10 to 3000 ms | Release time |
| **Knee** | 0 to 20 dB | Soft knee width |
| **Makeup** | -12 to 24 dB | Output gain compensation |
| **Mix** | 0 to 100% | Parallel compression blend |

## Building

### Prerequisites

- **CMake** 3.22 or higher
- **Xcode** (macOS) or **Visual Studio 2022** (Windows)
- **JUCE** 8.x (automatically cloned if not present)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/sobrinn/FIDI-comp.git
cd FIDI-comp/FIDIComp

# Clone JUCE (if not using submodule)
git clone https://github.com/juce-framework/JUCE.git

# Configure with CMake
cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release

# Build all formats (VST3, AU, Standalone)
cmake --build cmake-build --config Release -j8
```

### Output Locations

| Format | Location |
|--------|----------|
| **VST3** | `~/Library/Audio/Plug-Ins/VST3/FIDI Comp.vst3` |
| **AU** | `~/Library/Audio/Plug-Ins/Components/FIDI Comp.component` |
| **Standalone** | `cmake-build/FIDIComp_artefacts/Release/Standalone/` |

## Project Structure

```
FIDIComp/
├── CMakeLists.txt              # CMake build configuration
├── FIDIComp.jucer              # Projucer project (alternative build)
├── JUCE/                       # JUCE framework
└── Source/
    ├── PluginProcessor.cpp     # Audio routing & state management
    ├── PluginEditor.cpp        # GUI layout (700x340)
    ├── Compressor.cpp          # DSP: envelope follower & gain
    ├── Parameters.cpp          # Coefficient calculation
    ├── Meter.cpp               # Gain reduction visualization
    └── LookAndFeel.cpp         # Custom knob styling
```

## DSP Architecture

```
Input -> Stereo Link -> Envelope Follower -> Soft Knee Gain -> Mix -> Makeup -> Output
             |                                    |
        max(|L|,|R|)                       Smoothed GR -> Meter
```

**Key Design Decisions:**
- One-pole envelope follower with attack/release selection
- Quadratic soft knee interpolation for C1 continuity
- Batched parameter smoothing (every 32 samples)
- Atomic float for lock-free metering
- noexcept and nodiscard annotations for safety

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [JUCE Framework](https://juce.com/) - Cross-platform audio plugin framework
- Inspired by classic analog compressor designs

---

<div align="center">

**Made by FIDI**

</div>
