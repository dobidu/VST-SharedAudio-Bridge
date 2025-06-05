
#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include "JuceHeader.h"
#include "SharedMemoryManager.h"
#include "AudioFileReader.h" // Incluir o novo cabeçalho
#include "MusicAIProcessor.h"// Inclusão da classe "Ponte"

// Enum para os modos de geração de áudio
enum class AudioMode {
    Sine,
    File,
    MusicAI // Para o processamento com a lib
};

// Enum para os tipos de stem
enum class StemType {
    All,    // Todos os stems juntos (mixados)
    Vocals, // Apenas vocais
    Drums,  // Apenas bateria
    Bass,   // Apenas baixo
    Other   // Outros instrumentos
};

class SineWaveGenerator
{
public:
    SineWaveGenerator() :
        frequency(440.0f),
        isRunning(false),
        sharedMemory(),
        currentMode(AudioMode::Sine),
        currentStem(StemType::All),
        audioFileReader(std::make_unique<AudioFileReader>()),
        musicAIProcessor(std::make_unique<MusicAIProcessor>()),
        musicAIInitialized(false)
    {
        // Instance of SharedMemoryManager
        if (!sharedMemory.initialize())
        {
            std::cerr << "Falha ao inicializar a memoria compartilhada" << std::endl;
            return;
        }

        std::cout << "Memoria compartilhada inicializada com sucesso" << std::endl;

        // Inicializar buffers para os stems
        for (int i = 0; i < 4; i++) {
            stemBuffers.push_back(std::vector<float>());
        }
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

        switch (currentMode) {
        case AudioMode::Sine:
            std::cout << "Audio Generator Started (Senoid mode)" << std::endl;
            break;
        case AudioMode::File:
            std::cout << "Audio Generator Started (File mode)" << std::endl;
            break;
        case AudioMode::MusicAI:
            std::cout << "Audio Generator Started (MusicAI mode - "
                << getStemTypeName(currentStem) << ")" << std::endl;
            break;
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

        switch (currentMode) {
        case AudioMode::Sine:
            std::cout << "Audio Generator Stopped (Senoid mode)" << std::endl;
            break;
        case AudioMode::File:
            std::cout << "Audio Generator Stopped (File mode)" << std::endl;
            break;
        case AudioMode::MusicAI:
            std::cout << "Audio Generator Stopped (MusicAI mode)" << std::endl;
            break;
        }
    }

    bool loadAudioFile(const std::string& filePath)
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before loading a file." << std::endl;
            return false;
        }

        if (audioFileReader->openFile(filePath))
        {
            currentMode = AudioMode::File;

            // Updates the sample rate in the shared memory
            if (sharedMemory.isInitialized()) {
                //sharedMemory.setSampleRate(audioFileReader->getSampleRate());
                audioFileReader->setTargetSampleRate(sharedMemory.getSampleRate());
            }

            if (getCurrentMode() == AudioMode::MusicAI) {
                // inicializar o MusicAI com este arquivo
                initializeMusicAI(filePath);
            }

            return true;
        }

        return false;
    }

    bool initializeMusicAI(const std::string& audioFilePath)
    {
        // Verificar se o arquivo já está carregado
        if (!audioFileReader->isFileLoaded()) {
            if (!loadAudioFile(audioFilePath)) {
                return false;
            }
        }

        // Inicializar o processador MusicAI
        std::string modelPath = "models/vocals_others.mai";
        int sampleRate = static_cast<int>(sharedMemory.getSampleRate());

        if (!musicAIProcessor->initialize(modelPath, sampleRate)) {
            std::cerr << "Failed to initialize MusicAI processor" << std::endl;
            return false;
        }

        // Processar o arquivo de áudio para separar os stems
        if (!processAudioWithMusicAI()) {
            std::cerr << "Failed to process audio with MusicAI" << std::endl;
            return false;
        }

        musicAIInitialized = true;
        std::cout << "MusicAI initialized successfully" << std::endl;
        return true;
    }

    bool processAudioWithMusicAI()
    {
        if (!audioFileReader->isFileLoaded()) {
            std::cerr << "No audio file loaded" << std::endl;
            return false;
        }

        // Obter o áudio completo do arquivo
        std::vector<float> fileAudio;


        const int bufferSize = 8192;
        std::vector<float> tempBuffer(bufferSize);

        while (true) {
            int samplesRead = audioFileReader->getNextAudioBlock(tempBuffer.data(), bufferSize);
            if (samplesRead <= 0) break;

            fileAudio.insert(fileAudio.end(), tempBuffer.begin(), tempBuffer.begin() + samplesRead);

            if (samplesRead < bufferSize) break;
        }

        // Processar o áudio com MusicAI
        std::cout << "Processing audio with MusicAI..." << std::endl;

        // Limpar buffers de stems anteriores
        for (auto& buffer : stemBuffers) {
            buffer.clear();
        }

        // Processar em blocos para evitar problemas de memória com arquivos grandes
        const int processBlockSize = 8192;
        for (size_t offset = 0; offset < fileAudio.size(); offset += processBlockSize) {
            size_t blockSize = std::min(processBlockSize, static_cast<int>(fileAudio.size() - offset));

            if (!musicAIProcessor->processStereoAudio(
                fileAudio.data() + offset,
                blockSize / 2, // número de frames (cada frame tem 2 canais)
                stemBuffers)) {
                std::cerr << "Error processing audio block" << std::endl;
                return false;
            }
        }

        // Finalizar o processamento
        if (!musicAIProcessor->flush(stemBuffers)) {
            std::cerr << "Error flushing MusicAI processor" << std::endl;
            return false;
        }


        std::cout << "Audio processed successfully. Stems extracted." << std::endl;
        return true;
    }

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

        if (!musicAIInitialized) {
            std::cout << "MusicAI not initialized. Please load and process an audio file first." << std::endl;
            return;
        }

        currentMode = AudioMode::MusicAI;
        std::cout << "Switched to MusicAI mode - " << getStemTypeName(currentStem) << std::endl;
    }

    void setStemType(StemType stem)
    {
        if (isRunning.load())
        {
            std::cout << "Please stop the generator before changing stem type." << std::endl;
            return;
        }

        currentStem = stem;
        std::cout << "Stem type set to: " << getStemTypeName(currentStem) << std::endl;
    }

    std::string getStemTypeName(StemType stem) const
    {
        switch (stem) {
        case StemType::All: return "All (Mixed)";
        case StemType::Vocals: return "Vocals";
        case StemType::Drums: return "Drums";
        case StemType::Bass: return "Bass";
        case StemType::Other: return "Other Instruments";
        default: return "Unknown";
        }
    }

    // Gets valores atuais das variáveis de controle
    AudioMode getCurrentMode() const
    {
        return currentMode;
    }

    StemType getCurrentStem() const
    {
        return currentStem;
    }

    // verificações
    bool isFileLoaded() const
    {
        return audioFileReader->isFileLoaded();
    }

    bool isMusicAIInitialized() const
    {
        return musicAIInitialized;
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

        // Posição atual para reprodução dos stems
        size_t stemPosition = 0;

        // Timestamp for the last buffer sent
        auto lastBufferTime = std::chrono::high_resolution_clock::now();

        while (isRunning.load())
        {
            double currentSampleRate = sharedMemory.getSampleRate();
            if (currentSampleRate <= 0)
                currentSampleRate = 44100.0;  // Usar valor padrão se inválido

            if (currentMode == AudioMode::File && audioFileReader->isFileLoaded())
            {
                // Reads the audio data from the file
                int samplesRead = audioFileReader->getNextAudioBlock(buffer.data(), bufferSize);

                // If the end of the file is reached, loop back to the beginning
                if (samplesRead < bufferSize)
                {
                    std::fill(buffer.begin() + samplesRead, buffer.end(), 0.0f);

                }
            }
            else if (currentMode == AudioMode::MusicAI && musicAIInitialized)
            {
                // Reproduzir o stem selecionado
                int stemIndex;
                switch (currentStem) {
                case StemType::Vocals: stemIndex = 0; break;
                case StemType::Drums: stemIndex = 1; break;
                case StemType::Bass: stemIndex = 2; break;
                case StemType::Other: stemIndex = 3; break;
                case StemType::All: stemIndex = -1; break; // Todos os stems
                default: stemIndex = -1;
                }

                // Limpar o buffer
                std::fill(buffer.begin(), buffer.end(), 0.0f);

                if (stemIndex >= 0 && stemIndex < stemBuffers.size()) {
                    // Reproduzir apenas o stem selecionado
                    auto& stemBuffer = stemBuffers[stemIndex];

                    if (!stemBuffer.empty()) {
                        // Copiar dados do stem para o buffer de saída
                        for (int i = 0; i < bufferSize && stemPosition + i < stemBuffer.size(); i++) {
                            buffer[i] = stemBuffer[stemPosition + i];
                        }

                        // Avançar a posição
                        stemPosition += bufferSize;

                        // Loop se necessário
                        if (stemPosition >= stemBuffer.size()) {
                            stemPosition = 0;
                        }
                    }
                }
                else {
                    // Mixar todos os stems
                    for (int i = 0; i < bufferSize; i++) {
                        buffer[i] = 0.0f;

                        for (auto& stemBuffer : stemBuffers) {
                            if (!stemBuffer.empty() && stemPosition + i < stemBuffer.size()) {
                                buffer[i] += stemBuffer[stemPosition + i] * 0.25f; // Dividir por 4 para evitar clipping
                            }
                        }
                    }

                    // Avançar a posição
                    stemPosition += bufferSize;

                    // Loop se necessário
                    if (!stemBuffers.empty() && !stemBuffers[0].empty() && stemPosition >= stemBuffers[0].size()) {
                        stemPosition = 0;
                    }
                }
            }
            else // Modo Sine
            {
                // Senoid mode - uses the sample rate from the shared memory
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

    float frequency;                    // Senoid frequency in Hz
    std::atomic<bool> isRunning;        // Flag to indicate if the generator is running
    std::thread generatorThread;        // Thread for audio generation
    SharedMemoryManager sharedMemory;   // Shared memory manager instance
    AudioMode currentMode;              // Current audio mode (sine, file, or MusicAI)
    StemType currentStem;               // Current stem type for MusicAI mode
    std::unique_ptr<AudioFileReader> audioFileReader; // Audio file reader instance
    std::unique_ptr<MusicAIProcessor> musicAIProcessor; // MusicAI processor instance
    bool musicAIInitialized;            // Flag to indicate if MusicAI is initialized
    std::vector<std::vector<float>> stemBuffers; // Buffers for the separated stems
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

        // Opções baseadas no modo atual
        switch (generator.getCurrentMode()) {
        case AudioMode::Sine:
            std::cout << "1. Senoid mode: Start generation" << std::endl;
            break;

        case AudioMode::File:
            if (generator.isFileLoaded()) {
                std::cout << "1. Start file reproduction" << std::endl;
            }
            else {
                std::cout << "1. Start file reproduction (no file loaded)" << std::endl;
            }
            break;

        case AudioMode::MusicAI:
            if (generator.isMusicAIInitialized()) {
                std::cout << "1. Start MusicAI mode - "
                    << generator.getStemTypeName(generator.getCurrentStem()) << std::endl;
            }
            else {
                std::cout << "1. Start MusicAI mode (not initialized)" << std::endl;
            }
            break;
        }

        std::cout << "2. Stop generation/reproduction" << std::endl;

        if (generator.getCurrentMode() == AudioMode::Sine) {
            std::cout << "3. Frequency definition" << std::endl;
        }

        std::cout << "4. Load WAV file" << std::endl;

        // Opções de mudança de modo
        std::cout << "5. Switch mode" << std::endl;

        // Opções específicas do MusicAI
        if (generator.getCurrentMode() == AudioMode::MusicAI) {
            std::cout << "6. Select stem type" << std::endl;
        }

        std::cout << "7. Process current file with MusicAI" << std::endl;
        std::cout << "8. Exit" << std::endl;

        std::cout << "\nType the command number: ";
        int command;
        std::cin >> command;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (command)
        {
        case 1: // Start
            generator.start();
            break;

        case 2: // Stop
            generator.stop();
            break;

        case 3: // Frequency definition (Sine mode only)
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
                std::cout << "This command is only available in Sine mode." << std::endl;
            }
            break;
        }

        case 4: // Load WAV file
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

        case 5: // Switch mode
        {
            std::cout << "Select mode:\n";
            std::cout << "1. Sine Wave\n";
            std::cout << "2. Audio File\n";
            std::cout << "3. MusicAI\n";
            std::cout << "Selection: ";

            int modeChoice;
            std::cin >> modeChoice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            generator.stop(); // Stop before switching modes

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
                if (generator.isMusicAIInitialized()) {
                    generator.switchToMusicAIMode();
                }
                else {
                    std::cout << "MusicAI not initialized. Please process a file with MusicAI first." << std::endl;
                }
                break;

            default:
                std::cout << "Invalid selection." << std::endl;
                break;
            }
            break;
        }

        case 6: // Select stem type (MusicAI mode only)
        {
            if (generator.getCurrentMode() == AudioMode::MusicAI) {
                std::cout << "Select stem type:\n";
                std::cout << "1. All (Mixed)\n";
                std::cout << "2. Vocals\n";
                std::cout << "3. Drums\n";
                std::cout << "4. Bass\n";
                std::cout << "5. Other Instruments\n";
                std::cout << "Selection: ";

                int stemChoice;
                std::cin >> stemChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                generator.stop(); // Stop before changing stem type

                switch (stemChoice) {
                case 1: generator.setStemType(StemType::All); break;
                case 2: generator.setStemType(StemType::Vocals); break;
                case 3: generator.setStemType(StemType::Drums); break;
                case 4: generator.setStemType(StemType::Bass); break;
                case 5: generator.setStemType(StemType::Other); break;
                default: std::cout << "Invalid selection." << std::endl; break;
                }
            }
            else {
                std::cout << "This command is only available in MusicAI mode." << std::endl;
            }
            break;
        }

        case 7: // Process current file with MusicAI
        {
            if (generator.isFileLoaded()) {
                generator.stop(); // Stop before processing

                std::cout << "Processing file with MusicAI..." << std::endl;
                if (generator.initializeMusicAI("")) { // Empty string means use current file
                    std::cout << "File processed successfully. You can now switch to MusicAI mode." << std::endl;
                }
                else {
                    std::cout << "Failed to process file with MusicAI." << std::endl;
                }
            }
            else {
                std::cout << "No file loaded. Please load a file first." << std::endl;
            }
            break;
        }

        case 8: // Exit
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