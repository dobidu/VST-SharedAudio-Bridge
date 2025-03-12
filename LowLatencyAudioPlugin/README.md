# LowLatencyAudioPlugin

A VST3/Standalone audio plugin that receives audio data through shared memory from an external application with minimal latency. This plugin is part of the VST-SharedAudio-Bridge project, which demonstrates low-latency inter-process communication for audio applications.

## Features

- Receives audio data from external applications via shared memory
- Displays real-time latency measurement
- Shows connection status with external audio generators
- Monitors frequency information from the audio source
- Cross-platform (Windows, macOS, Linux)
- Available as both VST3 and Standalone application

## Code Structure

### Main Components

- **LowLatencyAudioPlugin**: Main audio processor class that handles audio processing
- **LowLatencyAudioProcessorEditor**: GUI component for the plugin
- **SharedMemoryManager**: Handles inter-process communication via shared memory

### Key Files

- `LowLatencyAudioPlugin.h/cpp`: Core plugin functionality
- `LowLatencyAudioProcessorEditor.h/cpp`: User interface implementation
- `SharedMemoryManager.h/cpp`: Cross-platform shared memory implementation
- `JuceHeader.h`: JUCE module includes and project settings
- `CMakeLists.txt`: CMake build configuration

## Building the Plugin

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (must be cloned at the same level as the plugin directory)

### Compilation Steps

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the plugin
cmake --build . --config Release
```

After successful compilation, the VST3 plugin and standalone application will be available in the `build/LowLatencyAudioPlugin_artefacts/` directory.

## Plugin Interface

The plugin interface consists of:

1. **Play/Stop Button**: Controls audio playback
2. **Status Indicator**: Shows "Connected" (green) when a generator is active or "Disconnected" (red) when no generator is detected
3. **Latency Display**: Shows the measured latency in milliseconds between data generation and playback
4. **Frequency Display**: Shows the current sine wave frequency received from the generator

## Using the Plugin

### In a DAW (Digital Audio Workstation)

1. Load the LowLatencyAudioPlugin.vst3 in your favorite DAW
2. Add the plugin to an audio track
3. Start the external audio generator application (e.g., SineWaveGenerator)
4. The plugin will automatically detect when audio data is being sent
5. You can stop and restart audio playback using the "Play/Stop" button

### As a Standalone Application

1. Launch the LowLatencyAudioPlugin standalone application
2. Start the external audio generator application
3. Audio will play automatically when data is received
4. Control playback using the "Play/Stop" button

## Technical Details

### Shared Memory Implementation

The plugin uses a custom cross-platform shared memory implementation that automatically adapts to different operating systems:

- **Windows**: Implements shared memory using `CreateFileMappingA` and `MapViewOfFile`
- **macOS/Linux**: Implements shared memory using POSIX `shm_open` and `mmap`

### Audio Processing

The audio processing workflow is as follows:

1. The external application writes audio data to shared memory
2. The plugin detects data availability and reads from shared memory
3. Timestamp comparison is used to calculate real-time latency
4. Audio data is routed to the plugin output
5. If connection is lost, playback is silenced

### Resilience Features

The plugin includes several resilience features:

- **Automatic reconnection**: Monitors for generator activity
- **Timeout detection**: Silences output if no data is received for a specific period
- **Data validity checking**: Ensures audio data integrity
- **Fallback mechanism**: Can reuse previous valid buffer if new data isn't available in time

## Performance Considerations

For optimal low-latency performance:

- Use small buffer sizes in your audio host (128 or 256 samples)
- Ensure the plugin and generator run on the same machine (local shared memory is faster)
- Close CPU-intensive applications to reduce system load
- Consider process priority settings on your operating system

## Extending the Plugin

Key areas for potential extension:

- **Multiple channels**: Extend to support stereo or multichannel audio
- **Additional parameters**: Add volume control, pan, or effects processing
- **Alternative IPC methods**: Implement socket-based or other communication methods
- **File loading**: Add support for reading audio files directly
- **Custom waveforms**: Support for waveforms beyond simple sine waves

## Troubleshooting

- If the plugin shows "Disconnected" despite the generator running, try restarting both applications
- If latency is unusually high, check your audio buffer settings and system load
- If you experience audio dropouts, consider increasing the buffer size slightly

## License

Copyright © 2025

This software is provided as is, without express or implied warranties.
