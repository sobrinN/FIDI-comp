cmake_minimum_required(VERSION 3.22)

project(FIDIComp VERSION 1.0.0)

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Create the plugin target
juce_add_plugin(FIDIComp
    COMPANY_NAME "FIDI"
    BUNDLE_ID "com.fidi.FIDIComp"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE FIDI
    PLUGIN_CODE FdCp
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "FIDI Comp"
)

# Generate JuceHeader.h
juce_generate_juce_header(FIDIComp)

# Add source files
target_sources(FIDIComp
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Compressor.cpp
        Source/Parameters.cpp
        Source/Meter.cpp
        Source/LookAndFeel.cpp
)

# Set C++ standard
target_compile_features(FIDIComp PUBLIC cxx_std_17)

# Link JUCE modules
target_link_libraries(FIDIComp
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions
target_compile_definitions(FIDIComp
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
