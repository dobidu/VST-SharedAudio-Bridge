//
// Created by gferraz on 01/08/2025.
//

#include "FileManagementPage.h"

//==============================================================================
FileManagementPage::FileManagementPage() : Component()
{
    setSize(355, 540);
    setBounds(0, 60, 355, 540);

    // Audio file components
    audioFileEditor = std::make_unique<juce::TextEditor>("AudioFileEditor");
    audioFileEditor->setBounds(20, 27, 315, 24);
    audioFileEditor->setCaretVisible(false);
    audioFileEditor->setReadOnly(true);
    audioFileEditor->setMultiLine(false);
    audioFileEditor->setText("No audio file loaded...");
    addAndMakeVisible(audioFileEditor.get());

    loadAudioFileButton.setButtonText("Load audio file");
    loadAudioFileButton.setBounds(20, 66, 315, 40);
    loadAudioFileButton.onClick = [this]() { loadAudioFile(); };
    addAndMakeVisible(loadAudioFileButton);

    // Model file components
    maiModelFileEditor = std::make_unique<juce::TextEditor>("ModelFileEditor");
    maiModelFileEditor->setBounds(20, 141, 315, 24);
    maiModelFileEditor->setCaretVisible(false);
    maiModelFileEditor->setReadOnly(true);
    maiModelFileEditor->setMultiLine(false);
    maiModelFileEditor->setText("No model file loaded...");
    addAndMakeVisible(maiModelFileEditor.get());

    loadModelFileButton.setButtonText("Load MusicAI model file");
    loadModelFileButton.setBounds(20, 180, 315, 40);
    loadModelFileButton.onClick = [this]() { loadModelFile(); };
    addAndMakeVisible(loadModelFileButton);

    validateModelButton.setButtonText("Validate model");
    validateModelButton.setBounds(20, 230, 315, 40);
    validateModelButton.setEnabled(false);
    validateModelButton.onClick = [this]() { validateModel(); };
    addAndMakeVisible(validateModelButton);

    modelValidationLabel.setBounds(20, 275, 315, 12);
    addAndMakeVisible(modelValidationLabel);

    // Buttons components
    processButton.setButtonText("Process audio");
    processButton.setBounds(20, 380, 315, 40);
    processButton.setEnabled(false);
    processButton.onClick = [this]() { launchProcessing(); };
    addAndMakeVisible(processButton);

    dumpProcessedFileButton.setButtonText("Dump processed file");
    dumpProcessedFileButton.setBounds(20, 430, 315, 40);
    dumpProcessedFileButton.setEnabled(false);
    dumpProcessedFileButton.onClick = [this]() { dumpProcessedAudioToFile(); };
    addAndMakeVisible(dumpProcessedFileButton);

    transmitButton.setButtonText("Transmit audio");
    transmitButton.setBounds(20, 480, 315, 40);
    // transmitButton.setColour(juce::Colours::green);
    transmitButton.setEnabled(false);
    transmitButton.onClick = [this]() { launchServer(); };
    addAndMakeVisible(transmitButton);

    // Progress bar
    processProgressMeter = std::make_unique<juce::ProgressBar>(processProgressPercent);
    processProgressMeter->setBounds(35, 358, 284, 15);
    addAndMakeVisible(processProgressMeter.get());
    processProgressMeter->setVisible(false);

    audioServer = std::make_unique<AudioServer>();
    formatManager.registerBasicFormats();
}

FileManagementPage::~FileManagementPage()
{}

void FileManagementPage::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void FileManagementPage::resized()
{}

// JUCE FILE FUNCTIONS ======================================================
void FileManagementPage::loadAudioFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose audio file",
        juce::File{},
        "*.wav;*.WAV"
    );
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
        {
            audioFileEditor->setText("Couldn't load file.");
            return;
        }
        auto* reader = formatManager.createReaderFor(file);
        if (reader->numChannels != 2)
        {
            audioFileEditor->setText("File with an incompatible number of channels (not stereo).");
            return;
        }

        juce::AudioBuffer<float> deinterleavedBuffer;
        int numOfChannels = reader->numChannels;
        int numOfSamples = reader->lengthInSamples;
        deinterleavedBuffer.setSize(numOfChannels, numOfSamples);
        reader->read(&deinterleavedBuffer, 0, numOfSamples, 0, true, true);

        interleaveSamples(deinterleavedBuffer, numOfChannels);
        readBufferMaxSamples = interleavedBuffer.size();

        audioFileEditor->setText(file.getFileName());
        delete reader;
    });
}

void FileManagementPage::loadModelFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose MusicAI model file",
        juce::File{},
        "*.mai;*.MAI"
    );
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
        {
            maiModelFileEditor->setText("Couldn't load file.");
            return;
        }
        maiModelFilePath = file.getFullPathName();
        maiModelFileEditor->setText(file.getFileName());
        modelValidationLabel.setText("", juce::dontSendNotification);
        validateModelButton.setEnabled(true);
    });
}

// MUSIC AI LIB FUNCTIONS ======================================================
void FileManagementPage::validateModel()
{
    musicai_processor_config config;
    memset(&config, 0, sizeof(config));
    config.frame_rate = 48000;
    config.preferred_accelerator_unit = MUSICAI_ACCELERATOR_UNIT_NPU;
    config.preferred_accelerator_unit_index = 0;
    config.preferred_queue_size = 80000;

    musicai* context = musicai_create();
    if (!context)
        return;

    musicai_status_result result = musicai_init(context, maiModelFilePath.getCharPointer(), &config);
    if (result.code != MUSICAI_STATUS_SUCCESS)
    {
        maiModelFileEditor->setText("");
        maiModelFilePath = "";
        validateModelButton.setEnabled(false);
        modelValidationLabel.setText("Invalid model. Please try another one.", juce::dontSendNotification);
        musicai_destroy(&context);
        return;
    }

    modelValidationLabel.setText("Valid model loaded.", juce::dontSendNotification);
    musicai_destroy(&context);
    processButton.setEnabled(true);
}

void FileManagementPage::launchProcessing()
{
    processProgressPercent = 0.0;
    processProgressMeter->setVisible(true);
    juce::Thread::launch([this] () { processAudioFile(); });

}

void FileManagementPage::processAudioFile() {
    // Prepare MusicAI for processing
    musicai_model_info model_info;
    musicai_status_result status_result = musicai_get_model_info_from_file(
        maiModelFilePath.getCharPointer(),
        &model_info
    );
    if (status_result.code != MUSICAI_STATUS_SUCCESS)
    {
        DBG("ERROR: Failed to get model info.");
        return;
    }

    musicai* context = musicai_create();
    if (!context)
    {
        DBG("ERROR: Failed to create context.");
        return;
    }

    musicai_processor_config config;
    config.frame_rate = 48000;
    config.preferred_accelerator_unit = MUSICAI_ACCELERATOR_UNIT_NPU;
    config.preferred_accelerator_unit_index = 0;
    config.preferred_queue_size = BLOCK_SIZE * 8;

    musicai_status_result result = musicai_init(
        context,
        maiModelFilePath.getCharPointer(),
        &config
    );
    if (result.code != MUSICAI_STATUS_SUCCESS)
    {
        DBG("ERROR: Failed to initialize context.");
        return;
    }

    // Init temp buffers for chunk processing
    juce::Array<float> tempInputBuffer;
    tempInputBuffer.resize(BLOCK_SIZE * NUM_CHANNELS);

    juce::Array<float> tempOutputBuffer;
    tempOutputBuffer.resize(BLOCK_SIZE * NUM_CHANNELS * NUM_OUTPUT_STEMS);

    // Do the processing
    size_t numFramesQueued = 0;
    size_t partialFramesQueued = 0;
    size_t numFramesRead = 0;
    size_t framesReadFromBuffer = 0;

    DBG("== START ==");
    do {
        tempInputBuffer.fill(0.f);
        framesReadFromBuffer = getBlockChunkFromBuffer(tempInputBuffer, BLOCK_SIZE);
        numFramesQueued = 0;

        do {
            result = musicai_process(
                context,
                tempInputBuffer.data() + numFramesQueued * NUM_CHANNELS,
                BLOCK_SIZE - numFramesQueued,
                &partialFramesQueued
            );
            if (result.code != MUSICAI_STATUS_SUCCESS) {
                DBG("ERROR: Failed to process input.");
                return;
            }
            numFramesQueued += partialFramesQueued;

            result = musicai_retrieve(
                context,
                tempOutputBuffer.data(),
                BLOCK_SIZE,
                &numFramesRead
            );
            if (result.code != MUSICAI_STATUS_SUCCESS) {
                DBG("ERROR: Failed to retrieve output.");
                return;
            }
            processProgressPercent = (double)readBufferChunkAccumulator / (double)readBufferMaxSamples;
            DBG("[PROCESS] Percent: " << processProgressPercent);

            processedBuffer.insertArray(
                -1,
                tempOutputBuffer.data(),
                numFramesRead * NUM_CHANNELS * NUM_OUTPUT_STEMS
            );
        } while (numFramesQueued < BLOCK_SIZE);
    } while (framesReadFromBuffer == BLOCK_SIZE);
    DBG("== END ==");

    musicai_flush(context);
    musicai_destroy(&context);
    processButton.setEnabled(false);
    dumpProcessedFileButton.setEnabled(true);
    transmitButton.setEnabled(true);
    processProgressMeter->setVisible(false);
}

// AUDIO SERVER FUNCTIONS =============================================
void FileManagementPage::launchServer()
{
    transmitButton.setButtonText("Stop server");
    // transmitButton.setColour()
    transmitButton.onClick = [this] () { stopServer(); };

    audioServer->setBuffer(processedBuffer);
    audioServer->setNumberOfSamples(processedBuffer.size());
    audioServer->startServer();
}

void FileManagementPage::stopServer()
{
    transmitButton.setButtonText("Start server");
    // transmitButton.setColour()
    transmitButton.onClick = [this] () { launchServer(); };
    audioServer->stopServer();
}

// AUX FUNCTIONS ======================================================
void FileManagementPage::interleaveSamples(const juce::AudioBuffer<float>& deinterleavedBuffer, int numChannels)
{
    int numSamples = deinterleavedBuffer.getNumSamples();
    interleavedBuffer.resize(numChannels * numSamples);

    for (int channel = 0; channel < numChannels; channel++) {
        const float* readPointer = deinterleavedBuffer.getReadPointer(channel);
        for (int n = 0; n < numSamples; n++)
        {
            int interleavedIndex = (n * numChannels) + channel;
            interleavedBuffer.set(interleavedIndex, readPointer[n]);
        }
    }
}

void FileManagementPage::deinterleaveSamples(juce::AudioBuffer<float>& deinterleavedBuffer, int numChannels)
{
    int numSamplesPerChannel = processedBuffer.size() / numChannels;
    deinterleavedBuffer.setSize(numChannels, numSamplesPerChannel);
    deinterleavedBuffer.clear();

    for (int channel = 0; channel < numChannels; channel++) {
        float* writer = deinterleavedBuffer.getWritePointer(channel);
        for (int n = 0; n < numSamplesPerChannel; n++)
        {
            int interleavedIndex = (n * numChannels) + channel;
            writer[n] = processedBuffer[interleavedIndex];
        }
    }
}

int FileManagementPage::getBlockChunkFromBuffer(juce::Array<float>& outputBuffer, int amountToFetch)
{
    auto blockSizeToCopy = amountToFetch * 2;
    if (readBufferChunkAccumulator + blockSizeToCopy > readBufferMaxSamples)
        return 0;

    juce::FloatVectorOperations::copy(
        outputBuffer.data(),
        interleavedBuffer.data() + readBufferChunkAccumulator,
        blockSizeToCopy
    );
    readBufferChunkAccumulator += blockSizeToCopy;
    return amountToFetch;
}

void FileManagementPage::dumpProcessedAudioToFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Save processed file",
        juce::File{},
        "*.wav"
    );
    auto chooserFlags = juce::FileBrowserComponent::saveMode;
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        juce::AudioBuffer<float> outputBuffer;
        deinterleaveSamples(outputBuffer, 8);

        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer;
        writer.reset(format.createWriterFor(
            new juce::FileOutputStream(file),
            48000.f,
            outputBuffer.getNumChannels(),
            24,
            {},
            0
        ));
        if (writer)
            writer->writeFromAudioSampleBuffer(outputBuffer, 0, outputBuffer.getNumSamples());
    });
}