#pragma once

#include "JuceHeader.h"
#include "SharedMemoryManager.h"

// Forward declaration
class LowLatencyAudioProcessorEditor;

//==============================================================================
class LowLatencyAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    LowLatencyAudioProcessor();
    ~LowLatencyAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock; // Necessário para evitar o warning

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void togglePlayback();
    bool isPlaying() const { return playing.load(); }
    float getCurrentLatency() const { return currentLatency.load(); }
    float getCurrentFrequency() const { return currentFrequency.load(); }

    bool isGeneratorActive() const { 
        return sharedMemory.isGeneratorActive() && !timeoutDetected.load(); 
    }
private:
    //==============================================================================
    SharedMemoryManager sharedMemory;
    std::atomic<bool> playing { false };
    juce::AudioBuffer<float> audioBuffer;
    std::atomic<float> currentLatency { 0.0f };
    std::atomic<float> currentFrequency { 440.0f };
    std::chrono::high_resolution_clock::time_point lastTimestamp;

    juce::AudioBuffer<float> lastBuffer;
    std::atomic<bool> hasValidData { false };

    std::chrono::time_point<std::chrono::high_resolution_clock> lastDataReceived;
    std::atomic<bool> timeoutDetected { false };
    static constexpr std::chrono::milliseconds dataTimeout { 500 }; // 500ms de timeout
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLatencyAudioProcessor)
};