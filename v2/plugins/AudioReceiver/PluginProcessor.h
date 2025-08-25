#pragma once

#include "ZMQCommClient.h"
#include "MemoryManager.h"
#include <juce_audio_processors/juce_audio_processors.h>

enum PluginState
{
    Ready,
    Connecting,
    Running,
    Disconnecting
};

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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
    void changePluginState(const PluginState newState);
    void changeChannelVolume(const int channel, const float newVolume);
    bool initMemoryManager();
    void closeMemoryManager();

    PluginState getPluginState() const;
    juce::AudioProcessorValueTreeState treeState;
private:
    //==============================================================================
    std::atomic<PluginState> pluginState;
    ZMQCommClient zmqClient;
    MemoryManager memoryManager;
    juce::Array<float> tempBuffer;
    juce::AudioBuffer<float> tempDeinterleavedBuffer;
    juce::Array<double> chVolume{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void deinterleaveSamples(int numSamples);
    void downmixBufferToOutput(juce::AudioBuffer<float>& buffer, int numOutputChannels, int numSamples);
    void setChannelsVolume();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};