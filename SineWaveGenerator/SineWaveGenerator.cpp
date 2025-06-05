#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <limits>
#include "JuceHeader.h"
#include "SharedMemoryManager.h"
#include "AudioFileReader.h" // Incluir o novo cabeçalho
#include "MusicAiProcessor.h" // Incluir o cabeçalho mediador para a biblioteca MusicAi

// Enum para os modos de geração de áudio
enum class AudioMode {
    Sine,
    File,
    MusicAI
};

class SineWaveGenerator
{
public:
    SineWaveGenerator() :
        frequency(440.0f),
        isRunning(false),
        sharedMemory(),
        currentMode(AudioMode::Sine),
        audioFileReader(std::make_unique<AudioFileReader>()),
        modelPath("models/vocals_others.mai"),
        outputPath("output.wav"),
        filePath(""),
        processor(),
        musicAIInitialized(false)
    {
        // Instance of SharedMemoryManager
        if (!sharedMemory.initialize())
        {
            std::cerr << "Falha ao inicializar a memoria compartilhada" << std::endl;
            return;
        }

        std::cout << "Memoria compartilhada inicializada com sucesso" << std::endl;
    }

    ~SineWaveGenerator()
    {
        stop();
        sharedMemory.setGeneratorActive(false);
    }

    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;

        // Atualizar a frequência na memória compartilhada
        if (sharedMemory.isInitialized()) {
            sharedMemory.setFrequency(newFrequency);
        }

        std::cout << "Frequency adjusted for " << frequency << " Hz" << std::endl;
    }

    void start()
    {
        if (isRunning.load())
            return;

        isRunning.store(true);

        // Indicates that the generator is active
        sharedMemory.setGeneratorActive(true);

        generatorThread = std::thread(&SineWaveGenerator::run, this);

        if (currentMode == AudioMode::Sine) {
            std::cout << "Audio Generator Started (Senoid mode)" << std::endl;
        }
        else if (currentMode == AudioMode::File) {
            std::cout << "Audio Generator Started (File mode)" << std::endl;
        }
        else if (currentMode == AudioMode::MusicAI) {
            std::cout << "Audio Generator Started (Music AI mode)" << std::endl;
        }
        else {
            std::cout << "Audio Generator Started" << std::endl;
        }
    }

    void stop()
    {
        if (!isRunning.load())
            return;

        isRunning.store(false);

        // Indicates that the generator is no longer active
        sharedMemory.setGeneratorActive(false);

        if (generatorThread.joinable())
            generatorThread.join();

        if (currentMode == AudioMode::Sine) {
            std::cout << "Audio Generator Stopped (Senoid mode)" << std::endl;
        }
        else if (currentMode == AudioMode::File) {
            std::cout << "Audio Generator Stopped (File mode)" << std::endl;
        }
        else {
            std::cout << "Audio Generator Stopped (Music AI mode)" << std::endl;
        }
    }

    bool loadAudioFile(const std::string& localFilePath)
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before loading a file." << std::endl;
            return false;
        }

        if (audioFileReader->openFile(localFilePath))
        {
            //currentMode = AudioMode::File;

            // Updates the sample rate in the shared memory
            if (sharedMemory.isInitialized()) {
                //sharedMemory.setSampleRate(audioFileReader->getSampleRate());
                audioFileReader->setTargetSampleRate(sharedMemory.getSampleRate());
            }
            std::cout << "Content successfully loaded from file: " << localFilePath << std::endl;
            filePath = localFilePath;
            return true;
        }

        return false;
    }

    // Mode switches
    void switchToSineMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }

        currentMode = AudioMode::Sine;
        std::cout << "Switched to Sine mode" << std::endl;
    }

    void switchToFileMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }

        currentMode = AudioMode::File;
        std::cout << "Switched to File mode" << std::endl;
    }

    void switchToMusicAIMode()
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before switching modes." << std::endl;
            return;
        }

        currentMode = AudioMode::MusicAI;
        std::cout << "Switched to Music AI mode" << std::endl;
    }

    // Music AI Methods 
    bool initializeMusicAI()
    {
        // Verificar se o arquivo já está carregado
        if (!audioFileReader->isFileLoaded()) {
            std::cerr << "Please upload a file first" << std::endl;
            return false;
        }

        std::cout << "MusicAI configuration prepared" << std::endl;

        // Consideramos inicializado com sucesso
        musicAIInitialized = true;
        return true;
    }

    void processByMusicAI() {
        try {
            // Verificar se está inicializado
            if (!musicAIInitialized) {
                if (!initializeMusicAI()) {
                    std::cerr << "Failed to initialize MusicAI" << std::endl;
                    return;
                }
            }
            
            // Processar o arquivo
            std::cout << "Processing file: " << filePath << std::endl;
            std::cout << "Using model: " << modelPath << std::endl;
            std::cout << "Output will be saved to: " << outputPath << std::endl;
            std::cout << "This may take a while, please wait..." << std::endl;

            // Chamar processFile com os parâmetros configurados
            bool success = processor.processFile(modelPath, filePath, outputPath);

            if (success) {
                std::cout << "Processing completed successfully. Output saved to: " << outputPath << std::endl;
            }
            else {
                std::cerr << "Processing failed " << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during MusicAI processing: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception during MusicAI processing" << std::endl;
        }
    }

    // status checkers
    AudioMode getCurrentMode() const
    {
        return currentMode;
    }

    bool isFileLoaded() const
    {
        return audioFileReader->isFileLoaded();
    }

private:
    void run()
    {
        // Configurations
        const int bufferSize = 4096;        // Audio buffer size
        float phase = 0.0f;                 // Senoid phase

        std::vector<float> buffer(bufferSize);

        // Keep track of the continuous phase for the sine wave
        float continuousPhase = 0.0f;

        // Timestamp for the last buffer sent
        auto lastBufferTime = std::chrono::high_resolution_clock::now();

        while (isRunning.load())
        {
            double currentSampleRate = sharedMemory.getSampleRate();;

            if (currentMode == AudioMode::MusicAI && audioFileReader->isFileLoaded()) {
                try {
                    processByMusicAI();
                }
                catch (...) {
                    std::cerr << "Unknown exception during MusicAI processing" << std::endl;
                }
                isRunning.store(false); // Parar após o processamento
                sharedMemory.setGeneratorActive(false);
                return;
            }
            else {
                if (currentMode == AudioMode::File && audioFileReader->isFileLoaded())
                {
                    // Uses the sample rate from the audio file
                    //currentSampleRate = audioFileReader->getSampleRate();


                    // Updates the sample rate in the shared memory
                    if (sharedMemory.isInitialized()) {
                        //sharedMemory.setSampleRate(currentSampleRate);
                        currentSampleRate = sharedMemory.getSampleRate();
                    }

                    // Reads the audio data from the file
                    int samplesRead = audioFileReader->getNextAudioBlock(buffer.data(), bufferSize);

                    // If the end of the file is reached, loop back to the beginning
                    if (samplesRead < bufferSize)
                    {
                        std::fill(buffer.begin() + samplesRead, buffer.end(), 0.0f);
                    }
                }
                else if (currentMode == AudioMode::Sine && audioFileReader->isFileLoaded())
                {
                    // Senoid mode - uses the sample rate from the shared memory
                    currentSampleRate = sharedMemory.getSampleRate();

                    if (currentSampleRate <= 0)
                        currentSampleRate = 44100.0;  // Usar valor padrão se inválido

                    float currentFrequency = frequency;

                    phase = continuousPhase; // Usar a fase continuada da iteração anterior

                    for (int i = 0; i < bufferSize; ++i)
                    {
                        buffer[i] = std::sin(phase);

                        phase += 2.0f * float(juce::MathConstants<double>::pi) * currentFrequency / static_cast<float>(currentSampleRate);

                        while (phase >= 2.0f * float(juce::MathConstants<double>::pi))
                            phase -= 2.0f * float(juce::MathConstants<double>::pi);
                    }

                    continuousPhase = phase;
                }
                else {
                    std::cerr << "Load a audio file " << std::endl;
                    isRunning.store(false); // Parar após o processamento
                    sharedMemory.setGeneratorActive(false);
                    return;
                }


                double bufferDurationMs = (bufferSize * 1000.0) / currentSampleRate;
                double targetRefreshMs = bufferDurationMs * 0.25; // 25% of the buffer duration

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

                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastBufferTime).count();

                if (elapsed < targetRefreshMs) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(
                        static_cast<int>(targetRefreshMs - elapsed)));
                }

                lastBufferTime = std::chrono::high_resolution_clock::now();
            }

        }
    }

    float frequency;                    // Senoid frequency in Hz
    std::atomic<bool> isRunning;        // Flag to indicate if the generator is running
    std::thread generatorThread;        // Thread for audio generation
    SharedMemoryManager sharedMemory;   // Shared memory manager instance
    AudioMode currentMode;              // Current audio mode (sine or file)
    std::unique_ptr<AudioFileReader> audioFileReader; // Audio file reader instance
    std::string filePath;
    std::string outputPath;
    std::string modelPath;
    MusicAIProcessor processor; // Music AI processor
    bool musicAIInitialized;
};

int main(int argc, char* argv[])
{
    std::cout << "Application for Low Latency VST Plugin Audio Generator" << std::endl;
    std::cout << "=====================================================================" << std::endl;

    SineWaveGenerator generator;

    // Menu interativo
    bool quit = false;
    while (!quit)
    {
        std::cout << "\nAvailable commands:" << std::endl;

        if (generator.getCurrentMode() == AudioMode::Sine) {
            std::cout << "1. Senoid mode: Start generation" << std::endl;
        }
        else if (generator.getCurrentMode() == AudioMode::File) {
            if (generator.isFileLoaded()) {
                std::cout << "1. Start file reproduction" << std::endl;
            }
            else {
                std::cout << "1. Start file reproduction (no file loaded)" << std::endl;
            }
        }
        else {
            if (generator.isFileLoaded()) {
                std::cout << "1. Start audio processing" << std::endl;
            }
            else {
                std::cout << "1. Start audio processing (no file loaded)" << std::endl;
            }
        }

        std::cout << "2. Stop generation/reproduction" << std::endl;

        if (generator.getCurrentMode() == AudioMode::Sine) {
            std::cout << "3. Frequency definition" << std::endl;
        }

        std::cout << "4. Load WAV file" << std::endl;

        std::cout << "5. Switch mode (current: ";
        switch (generator.getCurrentMode()) {
            case AudioMode::Sine:
                std::cout << "Sine)" << std::endl;
                break;
            case AudioMode::File:
                std::cout << "File)" << std::endl;
                break;
            case AudioMode::MusicAI:
                std::cout << "MusicAI)" << std::endl;
                break;
        }

        std::cout << "6. Exit" << std::endl;

        std::cout << "\nType the command number: ";

        int command;
        std::cin >> command;

        std::cin.ignore(10000, '\n');

        switch (command)
        {
        case 1:
            generator.start();
            break;

        case 2:
            generator.stop();
            break;

        case 3:
        {
            if (generator.getCurrentMode() == AudioMode::Sine) {
                float newFrequency;
                std::cout << "Digite a nova frequencia (Hz): ";
                std::cin >> newFrequency;

                if (newFrequency > 0 && newFrequency < 20000) {
                    generator.setFrequency(newFrequency);
                }
                else {
                    std::cout << "Invalid frequency. Please enter a value between 0 and 20,000 Hz." << std::endl;
                }
            }
            else {
                std::cout << "This command is not available in file mode." << std::endl;
            }
            break;
        }

        case 4:
        {
            std::string filePath;
            std::cout << "Enter the path to the WAV file: ";
            std::getline(std::cin, filePath);

            if (!filePath.empty()) {
                if (!generator.loadAudioFile(filePath)) {
                    std::cout << "Failure to load the file. Please check the path and try again." << std::endl;
                }
            }
            break;
        }

        case 5:
        {
            std::cout << "Select mode:" << std::endl;
            std::cout << "1. Sine Wave" << std::endl;
            std::cout << "2. File Playback" << std::endl;
            std::cout << "3. MusicAI Processing" << std::endl;
            std::cout << "Enter choice (1-3): ";

            int modeChoice;
            std::cin >> modeChoice;
            std::cin.ignore(10000, '\n');

            generator.stop(); // Parar antes de mudar o modo

            switch (modeChoice) {
            case 1:
                generator.switchToSineMode();
                break;
            case 2:
                if (generator.isFileLoaded()) {
                    generator.switchToFileMode();
                }
                else {
                    std::cout << "No file loaded. Please load a file first." << std::endl;
                }
                break;
            case 3:
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

        case 6:
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

