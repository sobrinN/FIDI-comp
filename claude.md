# FIDI Comp - VST3/AU Dynamics Compressor

## Overview

FIDI Comp is a professional-grade VST3/AU compressor plugin built with JUCE 8.x framework. Features stereo-linked compression with soft-knee, parallel mix (NY compression), real-time gain reduction metering, and a modern dark UI with glow effects.

## Project Structure

```
FIDIComp/
├── FIDIComp.jucer          # Projucer project file
├── CMakeLists.txt          # CMake build config
├── JUCE/                   # JUCE framework (git submodule)
└── Source/
    ├── PluginProcessor.*   # AudioProcessor - routing, state, APVTS
    ├── PluginEditor.*      # GUI - 700x340, two-row knob layout
    ├── Compressor.*        # DSP - envelope follower, soft-knee gain
    ├── Parameters.*        # Coefficient calculation, sample-rate aware
    ├── Meter.*             # Segmented LED meter with glow (30fps)
    └── LookAndFeel.*       # Custom rotary knobs with gradient arcs
```

## Parameters

| ID | Range | Default | Description |
|----|-------|---------|-------------|
| `threshold` | -60 to 0 dB | -20 | Compression threshold |
| `ratio` | 1:1 to 20:1 | 4:1 | Compression ratio (skew 0.5) |
| `attack` | 0.1-300 ms | 10 | Attack time (skew 0.4) |
| `release` | 10-3000 ms | 100 | Release time (skew 0.4) |
| `knee` | 0-20 dB | 6 | Soft knee width |
| `makeup` | -12 to 24 dB | 0 | Output gain |
| `mix` | 0-100% | 100 | Dry/wet blend |

## DSP Architecture

### Signal Flow
```
Input → Stereo Link (max) → Envelope Follower → Soft Knee → Parallel Mix → Makeup → Safety Check → Output
```

### Key Algorithms

**Envelope Follower** (linear domain, one-pole):
```cpp
coeff = (inputLevel > envelope) ? attackCoeff : releaseCoeff;
envelope = coeff * (envelope - inputLevel) + inputLevel;
```

**Coefficient Calculation** (decay-style):
```cpp
attackCoeff = exp(-1.0 / (sampleRate * attackMs * 0.001))  // ~0.99+ for slow
```

**Smoothing Coefficient** (speed-style):
```cpp
smoothingCoeff = 1.0 - exp(-1.0 / (sampleRate * 0.030))  // ~0.001 for 30ms
```

**Soft Knee** (quadratic interpolation):
- Below `threshold - halfKnee`: No compression
- Above `threshold + halfKnee`: Full ratio compression
- Inside knee: Interpolated effective ratio

### Safety Features
- NaN/Inf checks on gain reduction output
- Gain reduction clamped to [0.0, 1.0]
- Output sample NaN/Inf protection
- `juce::ScopedNoDenormals` in processBlock

## GUI Structure

### Layout (700x340)
```
┌─────────────────────────────────────────────────────┬─────┐
│  FIDI COMP                                     v1.0 │     │
├─────────────────────────────────────────────────────┤     │
│ COMPRESSION                                         │     │
│  [THR] [RAT] [ATK] [REL] [KNE]                     │ GR  │
│                                                     │ [█] │
│ OUTPUT                                              │ [█] │
│  [MKP] [MIX]                                       │ [█] │
└─────────────────────────────────────────────────────┴─────┘
```

### Color Scheme
- Background: `#0d0d1a` → `#1a1a2e` (gradient)
- Accent: `#00d4ff` (cyan)
- Text: White with 60%/99% opacity variants
- Meter: Cyan → Yellow → Orange → Red gradient

## Build

```bash
# Clone with JUCE submodule
git clone --recursive https://github.com/sobrinN/FIDI-comp.git
cd FIDI-comp/FIDIComp

# Build with CMake
cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release -j8
```

### Supported Sample Rates
- 44100 Hz, 48000 Hz, 96000 Hz (and others)
- All coefficients are sample-rate aware

## Output Locations

| Format | Path |
|--------|------|
| VST3 | `~/Library/Audio/Plug-Ins/VST3/FIDI Comp.vst3` |
| AU | `~/Library/Audio/Plug-Ins/Components/FIDI Comp.component` |
| Standalone | `cmake-build/FIDIComp_artefacts/Release/Standalone/` |

## Key Files Reference

| File | Lines | Purpose |
|------|-------|---------|
| `PluginProcessor.cpp` | 289 | Audio routing, per-sample processing loop |
| `Compressor.cpp` | 105 | Envelope + gain computation with safety |
| `Parameters.cpp` | 63 | Sample-rate aware coefficient calculation |
| `PluginEditor.cpp` | 173 | GUI layout, slider configuration |
| `Meter.cpp` | 140 | Segmented LED meter with glow effects |
| `LookAndFeel.cpp` | 155 | Custom rotary knob rendering |

## Dependencies

- **JUCE 8.x** (included as submodule)
- **CMake 3.22+**
- **Xcode** (macOS) or **Visual Studio 2022** (Windows)

## Thread Safety

- Parameters: Atomic reads from APVTS
- Metering: `std::atomic<float>` with compare-exchange
- GUI ↔ DSP: No shared mutable state except atomics
