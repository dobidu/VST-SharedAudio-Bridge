#pragma once

#include "JuceHeader.h"

// Forward declaration para quebrar a dependência circular
class LowLatencyAudioProcessor;

//==============================================================================
class LowLatencyAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit LowLatencyAudioProcessorEditor (LowLatencyAudioProcessor&);
    ~LowLatencyAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    //==============================================================================
    LowLatencyAudioProcessor& audioProcessor;

    juce::TextButton playButton;
    juce::Label latencyLabel;
    juce::Label latencyValueLabel;
    juce::Label frequencyLabel;      
    juce::Label frequencyValueLabel; 
    juce::Label statusLabel;
    juce::Label statusValueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLatencyAudioProcessorEditor)
};