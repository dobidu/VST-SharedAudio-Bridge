#pragma once

#include "JuceHeader.h"
#include <string>

// Classe para gerenciar a leitura de arquivos de áudio
class AudioFileReader
{
public:
    AudioFileReader();
    ~AudioFileReader();
    
    bool openFile(const std::string& filePath);
    void closeFile();
    bool isFileLoaded() const;
    
    int getNextAudioBlock(float* outputBuffer, int numSamples);
    
    double getSampleRate() const;
    void setTargetSampleRate(double rate);
    void resetPosition();
    
private:
    juce::AudioSampleBuffer audioData;
    double sampleRate;
    double targetSampleRate;  // plugin sample rate
    int numChannels;
    juce::int64 length;
    juce::int64 position;
    std::unique_ptr<juce::LagrangeInterpolator> resampler;
};