#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <limits>
#include "JuceHeader.h"
#include "SharedMemoryManager.h"
#include "AudioFileReader.h"
#include "MusicAiProcessor.h" // Mediator for MusicAI library integration

/**
 * @brief Enum defining the available audio generation modes
 */
enum class AudioMode {
    Sine,   // Sine wave generation mode
    File,   // Audio file playback mode
    MusicAI // AI-based audio processing mode
};

/**
 * @brief Main class for audio generation and processing
 *
 * This class handles generation of sine waves, playback of audio files,
 * and AI-based audio processing using the MusicAI library.
 */
class SineWaveGenerator
{
public:
    /**
     * @brief Constructor initializes the generator with default settings
     */
    SineWaveGenerator() :
        frequency(440.0f),                         // Default frequency for sine wave (A4)
        isRunning(false),                          // Generator starts in stopped state
        sharedMemory(),                            // Shared memory for VST communication
        currentMode(AudioMode::Sine),              // Default to sine wave generation
        audioFileReader(std::make_unique<AudioFileReader>()),
        modelPath("models/vocals_others.mai"),     // Path to MusicAI model file
        outputPath("./processed/"),                // Default output directory
        filePath(""),                              // No file loaded initially
        processor()                               // MusicAI processor
    {
        // Initialize shared memory for communication with VST plugin
        if (!sharedMemory.initialize())
        {
            std::cerr << "Failed to initialize shared memory" << std::endl;
            return;
        }
        std::cout << "Shared memory initialized successfully" << std::endl;
    }

    /**
     * @brief Destructor ensures proper cleanup
     */
    ~SineWaveGenerator()
    {
        stop();  // Stop audio generation
        sharedMemory.setGeneratorActive(false);  // Notify VST plugin
    }

    /**
     * @brief Sets the frequency for sine wave generation
     *
     * @param newFrequency Frequency in Hz for the sine wave
     */
    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        // Update frequency in shared memory for VST plugin
        if (sharedMemory.isInitialized()) {
            sharedMemory.setFrequency(newFrequency);
        }
        std::cout << "Frequency adjusted to " << frequency << " Hz" << std::endl;
    }

    /**
     * @brief Starts audio generation in the selected mode
     *
     * Creates and starts a thread for audio generation. For MusicAI mode,
     * the thread is joined immediately to wait for processing completion.
     */
    void start()
    {
        if (isRunning.load())
            return;  // Already running

        isRunning.store(true);
        // Notify VST plugin that generator is active
        sharedMemory.setGeneratorActive(true);

        // Create and start the audio processing thread
        generatorThread = std::thread(&SineWaveGenerator::run, this);

        // For MusicAI mode, wait for processing to complete
        if (currentMode == AudioMode::MusicAI) {
            // Wait for MusicAI processing to finish
            if (generatorThread.joinable()) {
                generatorThread.join();
            }

            // Clean up after processing is complete
            isRunning.store(false);
            sharedMemory.setGeneratorActive(false);
            std::cout << "Processing completed." << std::endl;

            // Ask user if they want to switch to File mode to play processed audio
            std::cout << "Do you want to switch to File Mode to play the processed file? (y/n): ";
            char response;
            std::cin >> response;
            std::cin.ignore(10000, '\n');

            if (response == 'y' || response == 'Y') {
                switchToFileMode();
            }
            std::cout << "Returning to menu." << std::endl;
        }
        else {
            // For other modes, thread continues in background
            if (currentMode == AudioMode::Sine) {
                std::cout << "Audio Generator Started (Sine wave mode)" << std::endl;
            }
            else if (currentMode == AudioMode::File) {
                std::cout << "Audio Generator Started (File playback mode)" << std::endl;
                std::cout << "Playing: " << filePath << std::endl;
            }
        }
    }

    /**
     * @brief Stops audio generation
     *
     * Signals the audio thread to stop and waits for it to complete.
     */
    void stop()
    {
        if (!isRunning.load())
            return;  // Already stopped

        isRunning.store(false);
        // Notify VST plugin that generator is inactive
        sharedMemory.setGeneratorActive(false);

        // Wait for audio thread to complete
        if (generatorThread.joinable()) {
            generatorThread.join();

            // Display appropriate message based on mode
            if (currentMode == AudioMode::Sine) {
                std::cout << "Audio Generator Stopped (Sine wave mode)" << std::endl;
            }
            else if (currentMode == AudioMode::File) {
                std::cout << "Audio Generator Stopped (File playback mode)" << std::endl;
            }
            else {
                std::cout << "Audio Generator Stopped (MusicAI mode)" << std::endl;
            }
        }
    }

    /**
     * @brief Loads an audio file for playback or processing
     *
     * @param localFilePath Path to the audio file to load
     * @return true if file was loaded successfully, false otherwise
     */
    bool loadAudioFile(const std::string& localFilePath)
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before loading a file." << std::endl;
            return false;
        }

        // Normalizar o caminho do arquivo
        std::string normalizedPath = normalizePath(localFilePath);

        if (audioFileReader->openFile(normalizedPath))
        {
            // Updates the sample rate in the shared memory
            if (sharedMemory.isInitialized()) {
                audioFileReader->setTargetSampleRate(sharedMemory.getSampleRate());
            }
            std::cout << "Content successfully loaded from file: " << normalizedPath << std::endl;
            filePath = normalizedPath;  // Armazenar o caminho normalizado
            return true;
        }
        else {
            std::cerr << "Failed to load file: " << normalizedPath << std::endl;
        }
        return false;
    }

    /**
     * @brief Switches to sine wave generation mode
     */
    void switchToSineMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }
        currentMode = AudioMode::Sine;
        std::cout << "Switched to Sine wave mode" << std::endl;
    }

    /**
     * @brief Switches to audio file playback mode
     */
    void switchToFileMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }
        currentMode = AudioMode::File;
        std::cout << "Switched to File playback mode" << std::endl;
    }

    /**
     * @brief Switches to MusicAI processing mode
     */
    void switchToMusicAIMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }
        currentMode = AudioMode::MusicAI;
        std::cout << "Switched to MusicAI processing mode" << std::endl;
    }

    /**
     * @brief Processes audio using MusicAI
     *
     * Separates audio into stems using the MusicAI library.
     */
    void processByMusicAI() {
        try {
            // Verify file is loaded
            if (!audioFileReader->isFileLoaded()) {
                std::cerr << "Please upload a file first" << std::endl;
                return;
            }

            // Generate output filename based on input file
            std::string fileName = extractFilename(filePath);
            outputPath = "processed_" + fileName;

            // Display processing information
            std::cout << "Processing file: " << filePath << std::endl;
            std::cout << "Using model: " << modelPath << std::endl;
            std::cout << "Output will be saved to: " << outputPath << std::endl;
            std::cout << "This may take a while, please wait..." << std::endl;

            // Process the file using MusicAI
            bool success = processor.processFile(modelPath, filePath, outputPath);

            if (success) {
                std::cout << "Processing completed successfully. Output saved to: " << outputPath << std::endl;

                // Update file path to point to processed file
                std::string originalFilePath = filePath;
                filePath = outputPath;
                std::cout << "File path updated from '" << originalFilePath << "' to '" << filePath << "'" << std::endl;
                return;
            }
            else {
                std::cerr << "Processing failed" << std::endl;
                return;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during MusicAI processing: " << e.what() << std::endl;
            return;
        }
        catch (...) {
            std::cerr << "Unknown exception during MusicAI processing" << std::endl;
            return;
        }
    }

    /**
     * @brief Normalizes a file path to use forward slashes in the format "/"
     *
     * @param path File path to be normalized
     * @return std::string normalized path
     */
    std::string normalizePath(const std::string& path) {
        std::string normalizedPath = path;
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        return normalizedPath;
    }

    /**
     * @brief Extracts filename from a full path
     * @param path Full path to file
     * @return std::string Filename without path
     */
    std::string extractFilename(const std::string& path) {
        // Find the last occurrence of slash or backslash
        size_t lastSlash = path.find_last_of("/\\");

        // If no slash found, return the entire path
        if (lastSlash == std::string::npos) {
            return path;
        }

        // Return substring after the last slash
        return path.substr(lastSlash + 1);
    }

    /**
     * @brief Returns the current audio mode
     * @return AudioMode Current mode (Sine, File, or MusicAI)
     */
    AudioMode getCurrentMode() const
    {
        return currentMode;
    }

    /**
     * @brief Checks if an audio file is loaded
     *
     * @return true if a file is loaded, false otherwise
     */
    bool isFileLoaded() const
    {
        return audioFileReader->isFileLoaded();
    }

private:
    /**
     * @brief Main audio processing function that runs in a separate thread
     *
     * Handles audio generation based on the current mode (Sine, File, or MusicAI).
     * For Sine mode, generates sine waves at the specified frequency.
     * For File mode, plays back the loaded audio file.
     * For MusicAI mode, processes the audio file using AI.
     */
    void run()
    {
        try {
            // Audio buffer configuration
            const int bufferSize = 4096;        // Size of audio buffer in samples
            float phase = 0.0f;                 // Phase for sine wave generation
            std::vector<float> buffer(bufferSize);  // Buffer for audio data
            float continuousPhase = 0.0f;       // Continuous phase for smooth sine wave

            // Timing for buffer updates
            auto lastBufferTime = std::chrono::high_resolution_clock::now();

            // Main processing loop
            while (isRunning.load())
            {
                try {
                    // Verify file is loaded for modes that require it
                    if (!audioFileReader->isFileLoaded() && currentMode != AudioMode::Sine) {
                        std::cerr << "No audio file loaded. Please load a file first." << std::endl;
                        isRunning.store(false);
                        sharedMemory.setGeneratorActive(false);
                        break;
                    }

                    // Get current sample rate from shared memory
                    double currentSampleRate = sharedMemory.getSampleRate();
                    if (currentSampleRate <= 0) {
                        currentSampleRate = 44100.0;  // Default if invalid
                    }

                    // Process based on current mode
                    if (currentMode == AudioMode::MusicAI && audioFileReader->isFileLoaded()) {
                        // Process audio with MusicAI
                        processByMusicAI();

                        // Signal completion and return
                        isRunning.store(false);
                        sharedMemory.setGeneratorActive(false);
                        return;
                    }
                    else if (currentMode == AudioMode::File && audioFileReader->isFileLoaded())
                    {
                        // Read audio data from file
                        int samplesRead = audioFileReader->getNextAudioBlock(buffer.data(), bufferSize);

                        // Handle end of file by filling remainder with silence
                        if (samplesRead < bufferSize)
                        {
                            std::fill(buffer.begin() + samplesRead, buffer.end(), 0.0f);
                        }
                    }
                    else if (currentMode == AudioMode::Sine)
                    {
                        // Generate sine wave
                        float currentFrequency = frequency;
                        phase = continuousPhase;  // Use continuous phase for smooth transition

                        for (int i = 0; i < bufferSize; ++i)
                        {
                            buffer[i] = std::sin(phase);
                            phase += 2.0f * float(juce::MathConstants<double>::pi) * currentFrequency / static_cast<float>(currentSampleRate);

                            // Keep phase in range [0, 2π)
                            while (phase >= 2.0f * float(juce::MathConstants<double>::pi))
                                phase -= 2.0f * float(juce::MathConstants<double>::pi);
                        }
                        continuousPhase = phase;  // Save phase for next buffer
                    }

                    // Write audio data to shared memory for VST plugin
                    double bufferDurationMs = (bufferSize * 1000.0) / currentSampleRate;
                    double targetRefreshMs = bufferDurationMs * 0.25;  // 25% of buffer duration

                    // Try to write data with exponential backoff
                    bool written = false;
                    int attempts = 0;
                    const int maxAttempts = 10;

                    while (!written && attempts < maxAttempts && isRunning.load()) {
                        written = sharedMemory.writeAudioData(buffer.data(), bufferSize);
                        if (!written) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1 << attempts));
                            attempts++;
                        }
                    }

                    // Maintain consistent timing
                    auto now = std::chrono::high_resolution_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - lastBufferTime).count();

                    if (elapsed < targetRefreshMs) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(
                            static_cast<int>(targetRefreshMs - elapsed)));
                    }

                    lastBufferTime = std::chrono::high_resolution_clock::now();
                }
                catch (const std::exception& e) {
                    std::cerr << "Exception in audio processing loop: " << e.what() << std::endl;
                    break;  // Exit loop on exception for safety
                }
                catch (...) {
                    std::cerr << "Unknown exception in audio processing loop" << std::endl;
                    break;  // Exit loop on unknown exception
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Fatal exception in run method: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown fatal exception in run method" << std::endl;
        }

        // Ensure generator is properly stopped
        isRunning.store(false);
        sharedMemory.setGeneratorActive(false);
    }

    // Core audio parameters
    float frequency;                                  // Frequency for sine wave generation (Hz)
    std::atomic<bool> isRunning;                      // Thread synchronization flag
    std::thread generatorThread;                      // Audio processing thread
    SharedMemoryManager sharedMemory;                 // Interface for VST plugin communication

    // Audio mode and file handling
    AudioMode currentMode;                            // Current audio generation/processing mode
    std::unique_ptr<AudioFileReader> audioFileReader; // Handles audio file loading and reading
    std::string filePath;                             // Path to current audio file

    // MusicAI processing
    MusicAIProcessor processor;                       // Interface to MusicAI library
    std::string outputPath;                           // Path for processed output files
    std::string modelPath;                            // Path to MusicAI model file
};

/**
 * @brief Main application entry point
 *
 * Provides an interactive command-line interface for audio generation,
 * playback, and AI-based processing.
 */
int main(int argc, char* argv[])
{
    std::cout << "Application for Low Latency VST Plugin Audio Generator" << std::endl;
    std::cout << "=====================================================================" << std::endl;

    // Create the main generator instance
    SineWaveGenerator generator;

    // Interactive command menu
    bool quit = false;
    while (!quit)
    {
        std::cout << "\nAvailable commands:" << std::endl;

        // Display appropriate start command based on current mode
        if (generator.getCurrentMode() == AudioMode::Sine) {
            std::cout << "1. Sine wave mode: Start generation" << std::endl;
        }
        else if (generator.getCurrentMode() == AudioMode::File) {
            if (generator.isFileLoaded()) {
                std::cout << "1. Start file playback" << std::endl;
            }
            else {
                std::cout << "1. Start file playback (no file loaded)" << std::endl;
            }
        }
        else {
            if (generator.isFileLoaded()) {
                std::cout << "1. Start audio processing with MusicAI" << std::endl;
            }
            else {
                std::cout << "1. Start audio processing (no file loaded)" << std::endl;
            }
        }

        std::cout << "2. Stop generation/playback" << std::endl;

        // Mode-specific commands
        if (generator.getCurrentMode() == AudioMode::Sine) {
            std::cout << "3. Set sine wave frequency" << std::endl;
        }

        std::cout << "4. Load WAV file" << std::endl;

        std::cout << "5. Switch mode (current: ";
        switch (generator.getCurrentMode()) {
        case AudioMode::Sine:
            std::cout << "Sine wave)" << std::endl;
            break;
        case AudioMode::File:
            std::cout << "File playback)" << std::endl;
            break;
        case AudioMode::MusicAI:
            std::cout << "MusicAI processing)" << std::endl;
            break;
        }

        std::cout << "6. Exit" << std::endl;

        // Get user command
        std::cout << "\nType the command number: ";
        int command;
        std::cin >> command;
        std::cin.ignore(10000, '\n');  // Clear input buffer

        // Process command
        switch (command)
        {
        case 1:  // Start generation/playback/processing
            generator.start();
            break;

        case 2:  // Stop generation/playback/processing
            generator.stop();
            break;

        case 3:  // Set frequency (Sine mode only)
        {
            if (generator.getCurrentMode() == AudioMode::Sine) {
                float newFrequency;
                std::cout << "Enter new frequency (Hz): ";
                std::cin >> newFrequency;

                // Validate frequency range
                if (newFrequency > 0 && newFrequency < 20000) {
                    generator.setFrequency(newFrequency);
                }
                else {
                    std::cout << "Invalid frequency. Please enter a value between 0 and 20,000 Hz." << std::endl;
                }
            }
            else {
                std::cout << "Frequency adjustment is only available in Sine wave mode." << std::endl;
            }
            break;
        }

        case 4:  // Load audio file
        {
            std::string filePath;
            std::cout << "Enter the path to the WAV file: ";
            std::getline(std::cin, filePath);

            if (!filePath.empty()) {
                if (!generator.loadAudioFile(filePath)) {
                    std::cout << "Failed to load the file. Please check the path and try again." << std::endl;
                }
            }
            break;
        }

        case 5:  // Switch mode
        {
            // Display mode selection menu
            std::cout << "Select mode:" << std::endl;
            std::cout << "1. Sine Wave" << std::endl;
            std::cout << "2. File Playback" << std::endl;
            std::cout << "3. MusicAI Processing" << std::endl;
            std::cout << "Enter choice (1-3): ";

            int modeChoice;
            std::cin >> modeChoice;
            std::cin.ignore(10000, '\n');

            // Stop any active generation before switching modes
            generator.stop();

            // Switch to selected mode
            switch (modeChoice) {
            case 1:  // Sine Wave
                generator.switchToSineMode();
                break;

            case 2:  // File Playback
                if (generator.isFileLoaded()) {
                    generator.switchToFileMode();
                }
                else {
                    std::cout << "No file loaded. Please load a file first." << std::endl;
                }
                break;

            case 3:  // MusicAI Processing
                if (generator.isFileLoaded()) {
                    generator.switchToMusicAIMode();
                }
                else {
                    std::cout << "No file loaded. Please load a file first." << std::endl;
                }
                break;

            default:
                std::cout << "Invalid choice." << std::endl;
                break;
            }
            break;
        }

        case 6:  // Exit application
            generator.stop();
            quit = true;
            break;

        default:
            std::cout << "Invalid command!" << std::endl;
            break;
        }
    }

    return 0;
}