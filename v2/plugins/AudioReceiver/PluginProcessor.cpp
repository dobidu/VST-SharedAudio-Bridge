#include "PluginProcessor.h"

#include <pluginterfaces/vst/vsttypes.h>

#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), pluginState(Connecting), zmqClient(), memoryManager(),
                        treeState(*this, nullptr, "PARAMS", createParameterLayout())
{

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.reserve(4);

    for (int i = 0; i < 4; ++i) {
        juce::String paramID = "ch" + std::to_string(i+1) + "vol";
        juce::String paramName = "Channel " + std::to_string(i+1) + " Volume";
        auto channelSlider = std::make_unique<juce::AudioParameterFloat>(paramID, paramName, 0.0, 1.f, 1.f);
        parameters.push_back(std::move(channelSlider));
    }

    return { parameters.begin(), parameters.end() };
}
//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG("Prepare to play");
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    DBG("Allocated " << samplesPerBlock * NUM_BUFFER_CHANNELS);
    tempBuffer.resize(samplesPerBlock * NUM_BUFFER_CHANNELS);
    tempDeinterleavedBuffer.setSize(NUM_BUFFER_CHANNELS, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int bufferSize = buffer.getNumSamples();

    for (auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, bufferSize);

    setChannelsVolume();

    if (pluginState == Running)
    {
        try
        {
            DBG("Resquested buffer size " << bufferSize);
            // TODO: Get start timestamp
            if (!zmqClient.requestAudioBlock(bufferSize))
            {
                DBG("Got no response from server.");
                zmqClient.refreshConnection();
            } else
            {
                DBG("Received server response. Copying " << bufferSize * 8 << " samples");
                memoryManager.copyShmToBuffer(tempBuffer.data(), bufferSize * 8);
                deinterleaveSamples(bufferSize);
                downmixBufferToOutput(buffer, totalNumOutputChannels, bufferSize);
                // TODO: Get stop timestamp
                // TODO: DBG start - stop, buffersize/samplerate
            }
        } catch (zmq::error_t& e)
        {
            DBG(e.what());
        }
    }
    if (pluginState == Disconnecting)
    {
        for (auto i = 0; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, bufferSize);
        changePluginState(Connecting);
    }
}

//==============================================================================
void AudioPluginAudioProcessor::deinterleaveSamples(int numSamples)
{
    int numChannels = tempDeinterleavedBuffer.getNumChannels();
    int numSamplesPerChannel = tempBuffer.size() / NUM_BUFFER_CHANNELS;

    for (int channel = 0; channel < numChannels; channel++) {
        float* writer = tempDeinterleavedBuffer.getWritePointer(channel);
        for (int n = 0; n < numSamples; n++)
        {
            int interleavedIndex = (n * numChannels) + channel;
            writer[n] = tempBuffer[interleavedIndex];
        }
    }
}

void AudioPluginAudioProcessor::setChannelsVolume() {
    int paramIdx = 1;
    for (int i = 0; i < NUM_BUFFER_CHANNELS; i += 2) {
        juce::String paramID = "ch" + std::to_string (paramIdx) + "vol";
        chVolume.set(i, *treeState.getRawParameterValue(paramID));
        chVolume.set(i + 1, *treeState.getRawParameterValue(paramID));
        paramIdx++;
    }
}

void AudioPluginAudioProcessor::downmixBufferToOutput(juce::AudioBuffer<float>& buffer, int numOutputChannels, int numSamples)
{
    for (int outputChannel = 0; outputChannel < numOutputChannels; ++outputChannel) {
        for (int inputChannel = 0; inputChannel < tempDeinterleavedBuffer.getNumChannels(); ++inputChannel) {
            if (inputChannel % numOutputChannels == outputChannel) {
                auto sourceReader = tempDeinterleavedBuffer.getReadPointer(inputChannel);

                float gain = chVolume[inputChannel];
                buffer.addFromWithRamp(outputChannel, 0, sourceReader, numSamples, gain, gain);
            }
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
void AudioPluginAudioProcessor::changeChannelVolume(const int channel, const float newVolume)
{
    if (channel >= 0 && channel < 5)
        chVolume.set(channel, newVolume);
}

void AudioPluginAudioProcessor::changePluginState(const PluginState newState)
{
    if (pluginState != newState)
        pluginState = newState;
}

PluginState AudioPluginAudioProcessor::getPluginState() const
{
    return pluginState;
}

bool AudioPluginAudioProcessor::initMemoryManager()
{
    return memoryManager.openMemoryBlockForReading();
}

void AudioPluginAudioProcessor::closeMemoryManager()
{
    memoryManager.closeMemoryBlock();
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}