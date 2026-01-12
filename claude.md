# FIDI Comp - Claude AI Reference

A professional VST3/AU dynamics compressor plugin built with JUCE 8.x. This document serves as the technical reference for AI-assisted development.

## Project Structure

```
FIDIComp/
├── CMakeLists.txt              # CMake build configuration
├── FIDIComp.jucer              # Projucer project file (alternative build)
├── JUCE/                       # JUCE framework (git submodule)
└── Source/
    ├── PluginProcessor.cpp/h   # AudioProcessor - routing, state, APVTS
    ├── PluginEditor.cpp/h      # GUI - 700x340, two-row knob layout
    ├── Compressor.cpp/h        # DSP - envelope follower, soft-knee gain
    ├── Parameters.cpp/h        # Coefficient calculation, sample-rate aware
    ├── Meter.cpp/h             # Segmented LED meter with glow (30fps)
    └── LookAndFeel.cpp/h       # Custom rotary knobs with gradient arcs
```

## Parameters

| ID          | Range            | Default | Unit  | Description                     |
|-------------|------------------|---------|-------|---------------------------------|
| `threshold` | -60 to 0         | -20     | dB    | Compression threshold           |
| `ratio`     | 1:1 to 20:1      | 4:1     | ratio | Compression ratio (skew 0.5)    |
| `attack`    | 0.1 to 300       | 10      | ms    | Attack time (skew 0.4)          |
| `release`   | 10 to 3000       | 100     | ms    | Release time (skew 0.4)         |
| `knee`      | 0 to 20          | 6       | dB    | Soft knee width                 |
| `makeup`    | -12 to 24        | 0       | dB    | Output gain                     |
| `mix`       | 0 to 100         | 100     | %     | Dry/wet blend                   |

## DSP Architecture

### Signal Flow

```
Input → Stereo Link (max) → Envelope Follower → Soft Knee → Parallel Mix → Makeup → Safety Check → Output
```

### Envelope Follower

Linear-domain one-pole filter with attack/release selection:

```cpp
double coeff = (inputLevel > envelope) ? attackCoeff : releaseCoeff;
envelope = coeff * (envelope - inputLevel) + inputLevel;
```

### Coefficient Calculation

Decay-style formula (sample-rate aware):

```cpp
attackCoeff = exp(-1.0 / (sampleRate * attackMs * 0.001))
```

### Batched Parameter Smoothing

Every 32 samples for CPU efficiency without audible artifacts:

```cpp
if (++smoothingCounter >= 32) {
    smoothingCounter = 0;
    smoothedParam += batchCoeff * (targetParam - smoothedParam);
}
```

### Soft Knee Algorithm

Quadratic interpolation for C1 continuity:

- **Below** `threshold - halfKnee`: No compression
- **Above** `threshold + halfKnee`: Full ratio compression
- **Inside knee**: `effectiveRatio = 1 + (ratio - 1) * kneeRatio²`

### Safety Features

- NaN/Inf checks on gain reduction output
- Gain reduction clamped to [0.0, 1.0]
- Output sample NaN/Inf protection
- `juce::ScopedNoDenormals` in processBlock

## GUI Architecture

### Layout (700 x 340)

```
┌─────────────────────────────────────────────────────┬─────┐
│  FIDI COMP                                    v1.0  │     │
├─────────────────────────────────────────────────────┤     │
│ COMPRESSION                                         │     │
│  [THR] [RAT] [ATK] [REL] [KNE]                     │ GR  │
│                                                     │ [█] │
│ OUTPUT                                              │ [█] │
│  [MKP] [MIX]                                       │ [█] │
└─────────────────────────────────────────────────────┴─────┘
```

### Color Palette

| Element          | Color                          |
|------------------|--------------------------------|
| Background       | `#0d0d1a` → `#1a1a2e` gradient |
| Accent           | `#00d4ff` (cyan)               |
| Accent Light     | `#80eaff`                      |
| Accent Glow      | `#00d4ff` (25% opacity)        |
| Text Primary     | `#ffffff`                      |
| Text Dim         | `#ffffff` (60% opacity)        |
| Track            | `#2a2a3e`                      |
| Meter Gradient   | Cyan → Yellow → Orange → Red   |

### Meter Implementation

- 16-segment LED-style visualization
- 30 FPS timer refresh rate
- Attack: ~50ms, Release: ~300ms ballistics
- Atomic float read/reset pattern for thread safety

## Build System

### CMake (Recommended)

```bash
git clone --recursive https://github.com/sobrinn/FIDI-comp.git
cd FIDI-comp/FIDIComp

cmake . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release -j8
```

### Requirements

- CMake 3.22+
- JUCE 8.x (git submodule)
- Xcode (macOS) or Visual Studio 2022 (Windows)
- C++17 standard

### Output Locations

| Format     | Path                                                      |
|------------|-----------------------------------------------------------|
| VST3       | `~/Library/Audio/Plug-Ins/VST3/FIDI Comp.vst3`           |
| AU         | `~/Library/Audio/Plug-Ins/Components/FIDI Comp.component` |
| Standalone | `cmake-build/FIDIComp_artefacts/Release/Standalone/`      |

### Supported Sample Rates

44100 Hz, 48000 Hz, 96000 Hz, and any other rate (all coefficients recalculated)

## Component Reference

| Component              | Responsibility                                          |
|------------------------|---------------------------------------------------------|
| `FIDICompProcessor`    | Audio routing, APVTS management, state serialization    |
| `FIDICompEditor`       | GUI layout, slider configuration, LookAndFeel           |
| `Compressor`           | Envelope + gain computation with batched smoothing      |
| `Parameters`           | Raw parameter reads, coefficient conversion             |
| `Meter`                | Timer-driven LED visualization with ballistics          |
| `FIDILookAndFeel`      | Custom rotary knobs, gradient arcs, glow effects        |

## Thread Safety

| Data                | Mechanism                                    |
|---------------------|----------------------------------------------|
| Parameters          | Atomic reads from APVTS (`std::atomic<float>`) |
| Gain Reduction      | `std::atomic<float>` with compare-exchange   |
| GUI ↔ DSP           | No shared mutable state except atomics       |

## Code Quality Standards

- `[[nodiscard]]` on all getter functions
- `noexcept` on performance-critical DSP methods
- `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` on all classes
- Consistent header guards with `#pragma once`
- Single compressor instance for stereo-linked operation

## Extension Points

### Adding New Parameters

1. Add parameter to `createParameterLayout()` in `PluginProcessor.cpp`
2. Add atomic reference in `Parameters.h`
3. Initialize reference in `Parameters.cpp` constructor
4. Add smoothed value and update logic in `Compressor` if needed
5. Add slider/attachment in `PluginEditor`

### Modifying DSP

- Core compression logic: `Compressor::computeGainReduction()`
- Knee curve: `Compressor::computeGainReductionDb()`
- Envelope timing: `Parameters::calculateCoefficient()`

### Customizing UI

- Colors: `FIDILookAndFeel` constructor and member variables
- Knob style: `FIDILookAndFeel::drawRotarySlider()`
- Layout: `FIDICompEditor::resized()`

## Skills Reference

The `/Skills/compressor-vst3/` directory contains comprehensive development guides:

- `SKILL.md` - Complete VST3 compressor development tutorial
- `references/dsp-algorithms.md` - Envelope followers, gain computation
- `references/juce-patterns.md` - AudioProcessor, APVTS, bus configuration  
- `references/gui-design.md` - Meters, knobs, LookAndFeel patterns
- `references/advanced-features.md` - Oversampling, sidechain, feedback, lookahead
