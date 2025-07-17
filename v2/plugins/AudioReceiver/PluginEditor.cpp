#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (300, 200);

    addAndMakeVisible(&connectBtn);
    connectBtn.setButtonText("Connect to Server");
    connectBtn.onClick = [this] { processorRef.changePluginState(Running); };

    addChildComponent(&disconnectBtn);
    disconnectBtn.setButtonText("Disconnect from Server");
    disconnectBtn.onClick = [this] { processorRef.changePluginState(Disconnecting); };

    addChildComponent(&playBtn);
    playBtn.setButtonText("Play");

    addChildComponent(&statusTxt);
    statusTxt.setFont(juce::FontOptions(20.0f));
    statusTxt.setJustificationType(juce::Justification::centred);
    statusTxt.setText("Ready to play", juce::dontSendNotification);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    PluginState pluginState = processorRef.getPluginState();
    if (pluginState == Running)
    {
        connectBtn.setVisible(false);
        disconnectBtn.setVisible(true);
        playBtn.setVisible(true);
        statusTxt.setVisible(true);
    } else
    {
        connectBtn.setVisible(true);
        disconnectBtn.setVisible(false);
        playBtn.setVisible(false);
        statusTxt.setVisible(false);
    }
}

void AudioPluginAudioProcessorEditor::resized()
{
    connectBtn.setBounds(100, getHeight() - 100, getWidth() - 200, 80);
    disconnectBtn.setBounds(20, getHeight() - 100, getWidth() - 200, 80);
    playBtn.setBounds(getWidth() - 120, getHeight() - 100, getWidth() - 200, 80);
    statusTxt.setBounds(0, 20, getWidth(), 20);
}