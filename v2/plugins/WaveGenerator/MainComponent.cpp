#include "MainComponent.h"


//==============================================================================
MainComponent::MainComponent() : TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto bgColor = findColour(MainComponent::backgroundColourId);

    setSize(355, 600);
    setBounds(getLocalBounds());
    addTab("Test", bgColor, new FileManagementPage(), true);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.drawText("MUSIC.AI STEM GENERATOR", 20, 30, 200, 15, juce::Justification::left, false);
    g.drawText("Beta 0.1.2", 20, 45, 100, 11, juce::Justification::left, false);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

//==============================================================================
// void MainComponent::stopServer()
// {
//     audioServer.stopServer();
//     changeState(WaitingForFile);
// }

