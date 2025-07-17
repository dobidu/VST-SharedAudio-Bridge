#pragma once

#include "AudioServer.h"
#include <JuceHeader.h>

enum ServerState
{
    WaitingForFile,
    WaitingForClient,
    Running,
    Stopping
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    juce::TextButton loadFileBtn;
    juce::TextButton stopServerBtn;
    juce::Label statusTxt;

    std::unique_ptr<juce::FileChooser>  fileChooser;
    juce::AudioFormatManager formatManager;
    ServerState state;

    void loadFile();
    void stopServer();
    void changeState(ServerState newState);

    AudioServer audioServer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
