#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    ch1Slider(juce::Slider::SliderStyle::LinearBarVertical, juce::Slider::TextEntryBoxPosition::NoTextBox),
    ch2Slider(juce::Slider::SliderStyle::LinearBarVertical, juce::Slider::TextEntryBoxPosition::NoTextBox),
    ch3Slider(juce::Slider::SliderStyle::LinearBarVertical, juce::Slider::TextEntryBoxPosition::NoTextBox),
    ch4Slider(juce::Slider::SliderStyle::LinearBarVertical, juce::Slider::TextEntryBoxPosition::NoTextBox)
{
    juce::ignoreUnused (processorRef);
    setSize (220, 280);

    ch1Slider.setBounds(20, 32, 15, 180);
    ch1Slider.setRange(0.f, 1.f, 0.05);
    ch1Slider.setValue(1);

    ch2Slider.setBounds(75, 32, 15, 180);
    ch2Slider.setRange(0.f, 1.f, 0.05);
    ch2Slider.setValue(1);

    ch3Slider.setBounds(130, 32, 15, 180);
    ch3Slider.setRange(0.f, 1.f, 0.05);
    ch3Slider.setValue(1);

    ch4Slider.setBounds(185, 32, 15, 180);
    ch4Slider.setRange(0.f, 1.f, 0.05);
    ch4Slider.setValue(1);

    ch1Attachment = std::make_unique<SliderAttachment>(processorRef.treeState, "ch1vol", ch1Slider);
    ch2Attachment = std::make_unique<SliderAttachment>(processorRef.treeState, "ch2vol", ch2Slider);
    ch3Attachment = std::make_unique<SliderAttachment>(processorRef.treeState, "ch3vol", ch3Slider);
    ch4Attachment = std::make_unique<SliderAttachment>(processorRef.treeState, "ch4vol", ch4Slider);

    addAndMakeVisible(ch1Slider);
    addAndMakeVisible(ch2Slider);
    addAndMakeVisible(ch3Slider);
    addAndMakeVisible(ch4Slider);

    receiverBtn.setBounds (10, 235, 90, 30);
    receiverBtn.setButtonText ("Receive");
    receiverBtn.onClick = [this] () { allowReceivingSamples(); };
    addAndMakeVisible(receiverBtn);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.drawText("Ch. 1", 6, 10, 42, 11, juce::Justification::centred, false);
    g.drawText("Ch. 2", 61, 10, 42, 11, juce::Justification::centred, false);
    g.drawText("Ch. 3", 116, 10, 42, 11, juce::Justification::centred, false);
    g.drawText("Ch. 4", 171, 10, 42, 11, juce::Justification::centred, false);

    g.drawText("MUSIC.AI", 109, 230, 100, 15, juce::Justification::right, false);
    g.drawText("STEM RECEIVER", 109, 245, 100, 15, juce::Justification::right, false);
    g.drawText("Beta 0.1.2", 109, 260, 100, 11, juce::Justification::right, false);
}

void AudioPluginAudioProcessorEditor::resized()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::allowReceivingSamples()
{
    if (!processorRef.initMemoryManager())
        return;

    processorRef.changePluginState(Running);
    receiverBtn.setButtonText("Stop");
    receiverBtn.onClick = [this] () { stopReceivingSamples(); };
}

void AudioPluginAudioProcessorEditor::stopReceivingSamples() {
    processorRef.changePluginState(Disconnecting);
    processorRef.closeMemoryManager();

    receiverBtn.setButtonText ("Receive");
    receiverBtn.onClick = [this] () { allowReceivingSamples(); };
}
