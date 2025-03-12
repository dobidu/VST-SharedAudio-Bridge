# SineWaveGenerator

A command-line application that generates sine wave audio data and sends it to the LowLatencyAudioPlugin through shared memory. This application is part of the VST-SharedAudio-Bridge project, which demonstrates low-latency inter-process communication for audio applications.

## Features

- Generates high-quality sine wave audio in real-time
- Transfers audio data to VST3 plugin via shared memory
- Adjustable frequency (1Hz - 20kHz)
- Displays connection status
- Cross-platform (Windows, macOS, Linux)
- Interactive command-line interface

## Code Structure

### Main Components

- **SineWaveGenerator**: Main class that handles sine wave generation and timing
- **SharedMemoryManager**: Handles inter-process communication via shared memory

### Key Files

- `SineWaveGenerator.cpp`: Contains the main application logic
- `SharedMemoryManager.h/cpp`: Cross-platform shared memory implementation
- `JuceHeader.h`: JUCE module includes for core functionality
- `CMakeLists.txt`: CMake build configuration

## Building the Application

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (only core modules needed)

### Compilation Steps

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the application
cmake --build . --config Release
```

After successful compilation, the executable will be available in the `build/` directory (Linux/macOS) or `build/Release/` directory (Windows).

## Using the Application

1. Start the LowLatencyAudioPlugin in your DAW or as a standalone application
2. Launch the SineWaveGenerator from the terminal or command prompt
3. The application presents an interactive menu with the following options:
   - `1`: Start audio generation
   - `2`: Stop audio generation
   - `3`: Change sine wave frequency
   - `4`: Exit the application

### Example Usage

```
Sine Wave Generator Application for Low Latency Audio Plugin
=====================================================

Commands available:
1. Start generation
2. Stop generation
3. Set frequency
4. Exit

Enter a command: 1
Sine wave generator started

Enter a command: 3
Enter new frequency (Hz): 880
Frequency adjusted to 880.0 Hz

Enter a command: 2
Sine wave generator stopped

Enter a command: 4
Exiting...
```

## Technical Details

### Audio Generation

The sine wave generation uses the standard C++ `std::sin()` function with precise phase accumulation to ensure continuous, glitch-free audio:

```cpp
// Generate sine wave with continuous phase
for (int i = 0; i < bufferSize; ++i) {
    buffer[i] = std::sin(phase);
    
    // Increment phase
    phase += 2.0f * float(juce::MathConstants<double>::pi) * currentFrequency / 
             static_cast<float>(currentSampleRate);
    
    // Keep phase between 0 and 2π
    while (phase >= 2.0f * float(juce::MathConstants<double>::pi))
        phase -= 2.0f * float(juce::MathConstants<double>::pi);
}
```

### Shared Memory Communication

The application uses a shared memory manager to transfer audio data to the plugin:

- Communicates with the plugin using a common shared memory segment
- Uses atomic variables for thread-safe communication
- Records timestamp information for latency measurement
- Monitors sample rate changes from the plugin

### Timing and Synchronization

The generator implements several techniques to ensure smooth audio delivery:

- Adaptive buffer timing based on sample rate
- Backoff strategy when the plugin is not ready to receive data
- Continuous phase tracking for seamless audio across buffer boundaries
- Sample rate synchronization with the plugin

## Performance Considerations

For optimal low-latency performance:

- Smaller buffer sizes (down to 256 samples) provide lower latency but increase CPU usage
- Higher frequencies require more precise timing to avoid aliasing
- The application automatically adjusts its timing based on the current sample rate
- Consider process priority settings on your operating system

## Extending the Application

Key areas for potential extension:

- **Additional waveforms**: Add support for square, sawtooth, triangle, or noise generators
- **Audio file playback**: Add support for reading and streaming audio files
- **Multiple channels**: Extend to support stereo or multichannel audio
- **Amplitude control**: Add volume/amplitude adjustment
- **Alternative IPC methods**: Implement socket-based or other communication methods

## Troubleshooting

- If the application fails to connect, make sure the plugin is running
- If you see "Failed to initialize shared memory," restart both applications
- If audio sounds distorted, try a lower frequency or check system load
- If the application seems unresponsive, it might be waiting for the plugin to read data

## License

Copyright © 2025

This software is provided as is, without express or implied warranties.
