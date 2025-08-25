//
// Created by gferraz on 01/08/2025.
//

#ifndef FILEMANAGEMENTPAGE_H
#define FILEMANAGEMENTPAGE_H

#include <JuceHeader.h>
#include <musicai/musicai.h>

#include "AudioServer.h"

#define NUM_CHANNELS 2
#define NUM_OUTPUT_STEMS 4
#define BLOCK_SIZE 8192

class FileManagementPage : public Component
{
public:
    //==============================================================================
    FileManagementPage();
    ~FileManagementPage() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Audio file components
    std::unique_ptr<juce::TextEditor> audioFileEditor;
    juce::TextButton loadAudioFileButton;

    // Model file components
    juce::String maiModelFilePath;
    std::unique_ptr<juce::TextEditor> maiModelFileEditor;
    juce::TextButton loadModelFileButton;
    juce::TextButton validateModelButton;
    juce::Label modelValidationLabel;

    juce::TextButton dumpProcessedFileButton;
    juce::TextButton processButton;
    juce::TextButton transmitButton;

    // Progress bar members
    double processProgressPercent{0.f};
    std::unique_ptr<juce::ProgressBar> processProgressMeter;

    // Aux members
    int readBufferChunkAccumulator{0};
    int readBufferMaxSamples{0};

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::AudioFormatManager formatManager;
    bool audioFileLoaded{false}, maiModelFileValidated{false}, processedFileReady{false};

    // Audio buffers
    juce::Array<float> interleavedBuffer;
    juce::Array<float> processedBuffer;

    std::unique_ptr<AudioServer> audioServer;

public:
    void loadAudioFile();
    void loadModelFile();
    void validateModel();
    void processAudioFile();

private:
    int getBlockChunkFromBuffer(juce::Array<float>& outputBuffer, int amountToFetch);
    void interleaveSamples(const juce::AudioBuffer<float>& deinterleavedBuffer, int numChannels);
    void deinterleaveSamples(juce::AudioBuffer<float>& deinterleavedBuffer, int numChannels);
    void launchProcessing();
    void launchServer();
    void stopServer();
    void dumpProcessedAudioToFile();
};

#endif //FILEMANAGEMENTPAGE_H
