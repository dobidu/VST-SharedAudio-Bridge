# Low Latency Audio System with JUCE

This repository contains a low latency audio system developed with JUCE, consisting of two applications:

1. **LowLatencyAudioPlugin**: A VST3/Standalone audio plugin that plays audio received via shared memory
2. **SineWaveGenerator**: A sine wave generator application that sends audio data to the plugin

The system uses inter-process shared memory to transfer audio data with minimal latency, measuring and displaying this latency in real-time.

## Project Structure

```
.
├── JUCE/                         # JUCE library (to be cloned)
├── LowLatencyAudioPlugin/        # VST3/Standalone Plugin Code
│   ├── CMakeLists.txt
│   ├── JuceHeader.h
│   ├── LowLatencyAudioPlugin.cpp
│   ├── LowLatencyAudioPlugin.h
│   ├── LowLatencyAudioProcessorEditor.cpp
│   ├── LowLatencyAudioProcessorEditor.h
│   ├── SharedMemoryManager.cpp
│   └── SharedMemoryManager.h
└── SineWaveGenerator/            # Sine Wave Generator Code
    ├── CMakeLists.txt
    ├── JuceHeader.h
    ├── SharedMemoryManager.cpp
    ├── SharedMemoryManager.h
    └── SineWaveGenerator.cpp
```

## Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- Git

## External Dependencies

### JUCE Framework

This project depends on JUCE, which must be cloned in the root folder of the project (at the same level as the plugin and generator folders):

```bash
git clone https://github.com/juce-framework/JUCE.git
```

In the future, JUCE will be added as a Git submodule to facilitate version management.

## Compilation

The two applications need to be compiled separately.

### Compiling the VST3/Standalone Plugin

```bash
# Create build directory for the plugin
cd LowLatencyAudioPlugin
mkdir build
cd build

# Configure the project with CMake
cmake ..

# Compile the project
cmake --build . --config Release

# The plugin files will be in the build/LowLatencyAudioPlugin_artefacts/ folder
```

### Compiling the Sine Wave Generator

```bash
# Create build directory for the generator
cd SineWaveGenerator
mkdir build
cd build

# Configure the project with CMake
cmake ..

# Compile the project
cmake --build . --config Release

# The executable will be in the build/ folder (on Linux/MacOS systems)
# or build/Release/ (on Windows)
```

## Supported Platforms

- Windows
- macOS
- Linux

The code includes specific handling for each operating system, particularly for the shared memory implementation.

## Using the System

### Step by Step

1. **Start the VST3 Plugin or Standalone Version**
   - Load the VST3 plugin in an audio host (DAW) or
   - Run the Standalone version of the plugin

2. **Start the Sine Wave Generator**
   - Run the SineWaveGenerator application from the terminal or file explorer

3. **Generator Control**
   - In the SineWaveGenerator application, select the following options:
     - `1` to start audio generation
     - `2` to stop generation
     - `3` to change the sine wave frequency
     - `4` to exit the application

4. **Plugin Control**
   - The plugin automatically detects when the generator is active and starts to play whenever the information is sent.
   - You can stop playback at any time using the "Stop" button (even when the generator is active), and you need to press the "Play" button in the plugin to restart playback.
   - The interface will show:
     - Connection status with the generator
     - Current latency in milliseconds
     - Current sine wave frequency

### Operation Details

- The system uses inter-process shared memory to transfer audio samples
- Latency is measured by comparing timestamps at generation and playback time
- The plugin automatically detects when the generator is active or inactive
- If the connection is lost, the plugin indicates "Disconnected" and silences the audio output

## Technical Details

### Shared Memory

The system uses a custom shared memory implementation that works on Windows, macOS, and Linux:

- Windows: Uses `CreateFileMappingA` and `MapViewOfFile`
- macOS/Linux: Uses `shm_open` and `mmap`

### Shared Data Structure

```cpp
struct AudioSharedData {
    static constexpr int maxBufferSize = 16384;  // Maximum buffer size
    
    std::atomic<int> readPosition { 0 };
    std::atomic<int> writePosition { 0 };
    std::atomic<bool> dataReady { false };
    std::atomic<int> bufferSize { 0 };
    std::atomic<double> sampleRate { 44100.0 };
    std::atomic<uint64_t> timestamp { 0 }; 
    std::atomic<float> frequency { 440.0f }; 
    std::atomic<bool> generatorActive { false };
    float audioData[maxBufferSize];
};
```

This structure stores:
- Audio data (samples)
- Control information (read/write positions)
- Synchronization information (timestamp for latency calculation)
- Parameters (frequency, sample rate)
- Generator state (active/inactive)

## Troubleshooting

### Connection Issues

- **Symptom**: Plugin displays "Disconnected" even with the generator running
- **Solution**: Restart both applications, starting with the plugin first and then the generator

### High Latency

- **Symptom**: Displayed latency above 50ms
- **Solution**: 
  - Check if other applications are consuming too many system resources
  - Reduce the buffer size in the DAW (if using the VST3 plugin)
  - Check system audio settings

### Compilation Errors

- **Issue**: Error related to JUCE not found
- **Solution**: Make sure the JUCE repository was properly cloned in the root folder

## Future Development

- Add support for multiple audio channels
- Implement adjustable buffer settings
- Improve the user interface
- Add more waveforms beyond sine wave
- Add wave file support
- Add different IPC methods for comparison
- Integrate JUCE as a Git submodule

## License

Copyright © 2025

This software is provided as is, without express or implied warranties.

## Contact

For questions, suggestions or contributions, please open an issue in this repository.
