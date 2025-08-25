#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    juce::Slider ch1Slider, ch2Slider, ch3Slider, ch4Slider;
    std::unique_ptr<SliderAttachment> ch1Attachment, ch2Attachment, ch3Attachment, ch4Attachment;

    juce::TextButton receiverBtn;
    juce::TextButton playBtn;

    //==============================================================================
    void allowReceivingSamples();
    void stopReceivingSamples();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};