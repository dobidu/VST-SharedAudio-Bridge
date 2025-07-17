#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : audioServer()
{
    state = WaitingForFile;

    addAndMakeVisible(&loadFileBtn);
    loadFileBtn.setButtonText("Load File");
    loadFileBtn.onClick = [this] { loadFile(); };

    addChildComponent(&stopServerBtn);
    stopServerBtn.setButtonText("Stop Server");
    stopServerBtn.onClick = [this] { stopServer(); };

    addChildComponent(&statusTxt);
    statusTxt.setFont(juce::FontOptions(20.0f));
    statusTxt.setJustificationType(Justification::centred);
    statusTxt.setText("Waiting for client...", juce::dontSendNotification);

    formatManager.registerBasicFormats();
    setSize (300, 200);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    loadFileBtn.setBounds(100, getHeight() - 100, getWidth() - 200, 80);
    stopServerBtn.setBounds(100, getHeight() - 100, getWidth() - 200, 80);
    statusTxt.setBounds(0, 20, getWidth(), 20);
}

//==============================================================================
void MainComponent::stopServer()
{
    audioServer.stopServer();
}

void MainComponent::loadFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select File",
        juce::File{},
        "*.wav;*.WAV"
    );

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            auto* reader = formatManager.createReaderFor(file);
            juce::AudioBuffer<float>& audioBuffer = audioServer.getBuffer();
            audioServer.setNumberOfSamples(reader->lengthInSamples);
            audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
            reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);
            delete reader;
        }
        audioServer.startServer();
        changeState(Running);
    }
    );

}

void MainComponent::changeState(ServerState newState)
{
    if (newState != state)
    {
        state = newState;
        switch (newState)
        {
            case WaitingForFile:
                loadFileBtn.setVisible(true);
                stopServerBtn.setVisible(false);
                statusTxt.setVisible(false);
                break;
            case WaitingForClient:
                loadFileBtn.setVisible(false);
                stopServerBtn.setVisible(true);
                stopServerBtn.setEnabled(false);
                statusTxt.setVisible(true);
                break;
            case Running:
                loadFileBtn.setVisible(false);
                stopServerBtn.setVisible(true);
                stopServerBtn.setEnabled(true);
                statusTxt.setVisible(false);
                statusTxt.setText("Ready to play!", juce::dontSendNotification);
                break;
            case Stopping:
                loadFileBtn.setVisible(true);
                stopServerBtn.setVisible(false);
                statusTxt.setVisible(false);
                break;
        }
    }
}

