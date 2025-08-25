#pragma once

#include "AudioServer.h"
#include "FileManagementPage.h"
#include <JuceHeader.h>

enum ServerState
{
    WaitingForFile,
    WaitingForClient,
    Running,
    Stopping
};

//==============================================================================
class MainComponent : public juce::TabbedComponent
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
    juce::AudioBuffer<float> audioBuffer;

    // void stopServer();
    // void changeState(ServerState newState);

    // AudioServer audioServer;
    // ServerState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
