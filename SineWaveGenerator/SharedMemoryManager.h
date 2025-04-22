#pragma once

#include "JuceHeader.h"
#include <atomic>
#include <chrono>
#include <string>
#include <mutex>

// Definição da estrutura de dados na memória compartilhada
struct AudioSharedData {
    static constexpr int maxBufferSize = 16384;  
    
    std::atomic<int> readPosition { 0 };
    std::atomic<int> writePosition { 0 };
    std::atomic<bool> dataReady { false };
    std::atomic<int> bufferSize { 0 };
    std::atomic<double> sampleRate { 44100.0 };
    std::atomic<double> originalSampleRate { 44100.0 }; 
    std::atomic<uint64_t> timestamp { 0 }; 
    std::atomic<float> frequency { 440.0f }; 
    std::atomic<bool> generatorActive { false };  
    float audioData[maxBufferSize];

};

class SharedMemoryManager
{
public:
    SharedMemoryManager();
    ~SharedMemoryManager();

    bool initialize();
    bool isInitialized() const { return initialized; }
    
    // Para o plugin VST (cliente)
    bool readAudioData(juce::AudioBuffer<float>& buffer, int numSamples, float& latencyMs);
    
    // Para a aplicação externa (servidor)
    bool writeAudioData(const float* data, int numSamples);
    void setSampleRate(double newSampleRate);
    double getSampleRate() const;

    // Métodos para frequência
    void setFrequency(float newFrequency) {
        if (initialized && sharedData != nullptr) {
            std::unique_lock<std::mutex> lock(accessMutex);
            sharedData->frequency.store(newFrequency);
        }
    }
    
    float getFrequency() const {
        if (initialized && sharedData != nullptr) {
            return sharedData->frequency.load();
        }
        return 440.0f; // valor padrão
    }

    // Métodos para indicar status do gerador
    void setGeneratorActive(bool active) {
        if (initialized && sharedData != nullptr) {
            std::unique_lock<std::mutex> lock(accessMutex);
            sharedData->generatorActive.store(active);
        }
    }
    
    bool isGeneratorActive() const {
        if (initialized && sharedData != nullptr) {
            return sharedData->generatorActive.load();
        }
        return false;
    }
    
private:
    // Implementação multiplataforma de memória compartilhada
    class PlatformSharedMemory {
    public:
        PlatformSharedMemory(const std::string& name, size_t size);
        ~PlatformSharedMemory();
        
        void* getData() { return data; }
        bool isValid() const { return isCreated; }
        
    private:
        std::string memoryName;
        void* data;
        size_t memSize;
        bool isCreated;
        bool isOwner;

    #if JUCE_WINDOWS
        void* fileHandle;
        void* mapHandle;
    #elif JUCE_MAC || JUCE_LINUX
        int fileDescriptor;
    #endif
    };
    
    std::unique_ptr<PlatformSharedMemory> sharedMemoryBlock;
    AudioSharedData* sharedData;
    bool initialized;
    std::mutex accessMutex;
    
    static constexpr const char* sharedMemoryName = "LowLatencyAudioPluginSharedMemory";
    static constexpr int sharedMemorySize = sizeof(AudioSharedData);
};