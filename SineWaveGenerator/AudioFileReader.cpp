#include "AudioFileReader.h"
#include <iostream>

AudioFileReader::AudioFileReader() 
    : sampleRate(44100.0), targetSampleRate(44100.0), numChannels(0), length(0), position(0)
{
    resampler = std::make_unique<juce::LagrangeInterpolator>();
}


AudioFileReader::~AudioFileReader()
{
    closeFile();
}

bool AudioFileReader::openFile(const std::string& filePath)
{
    closeFile();
    
    juce::File file(filePath);
    
    if (!file.existsAsFile())
    {
        std::cerr << "File not found: " << filePath << std::endl;
        return false;
    }
    
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats(); 
     
    juce::AudioFormatReader* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
    {
        std::cerr << "'File format not supported': " << filePath << std::endl;
        return false;
    }

    std::unique_ptr<juce::AudioFormatReader> formatReader(reader);
    
    sampleRate = formatReader->sampleRate;
    numChannels = formatReader->numChannels;
    length = formatReader->lengthInSamples;
    
    audioData.setSize(numChannels, static_cast<int>(length));
    formatReader->read(&audioData, 0, static_cast<int>(length), 0, true, true);
    
    position = 0;
    
    std::cout << "Open file: " << filePath << std::endl;
    std::cout << "Sample rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "Channels: " << numChannels << std::endl;
    std::cout << "Duration: " << length / sampleRate << " secs" << std::endl;
    
    return true;
}

void AudioFileReader::closeFile()
{
    audioData.clear();
    numChannels = 0;
    length = 0;
    position = 0;
}

bool AudioFileReader::isFileLoaded() const
{
    return length > 0 && numChannels > 0;
}

void AudioFileReader::setTargetSampleRate(double rate)
{
    targetSampleRate = rate;
    std::cout << "Target sample rate set to: " << targetSampleRate << " Hz" << std::endl;
}

int AudioFileReader::getNextAudioBlock(float* outputBuffer, int numSamples)
{
    if (!isFileLoaded() || position >= length)
        return 0;
    
    // Verificar se precisamos fazer resampling
    if (std::abs(sampleRate - targetSampleRate) > 0.01)
    {
        // Calcular a proporção de taxa de amostragem
        double ratio = sampleRate / targetSampleRate;
        
        // Buffer temporário para amostras de origem (com um pouco de margem)
        int sourceSamplesToRead = static_cast<int>(numSamples * ratio) + 8;
        sourceSamplesToRead = juce::jmin(sourceSamplesToRead, static_cast<int>(length - position));
        
        if (sourceSamplesToRead <= 0)
            return 0;
        
        // Criar buffer temporário para as amostras de origem
        juce::HeapBlock<float> sourceBuffer(sourceSamplesToRead);
        
        // Preencher o buffer de origem com amostras do arquivo
        if (numChannels > 1)
        {
            // Mixdown para mono
            for (int i = 0; i < sourceSamplesToRead; ++i)
            {
                float sample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    sample += audioData.getSample(ch, static_cast<int>(position) + i);
                }
                sourceBuffer[i] = sample / static_cast<float>(numChannels);
            }
        }
        else if (numChannels == 1)
        {
            // Copiar diretamente do canal mono
            for (int i = 0; i < sourceSamplesToRead; ++i)
            {
                sourceBuffer[i] = audioData.getSample(0, static_cast<int>(position) + i);
            }
        }
        
        // Usar interpolação linear básica em vez do LagrangeInterpolator
        // Esta abordagem é mais simples e evita os problemas com a API do JUCE
        for (int i = 0; i < numSamples; ++i)
        {
            double sourcePos = i * ratio;
            int sourcePos1 = static_cast<int>(sourcePos);
            int sourcePos2 = sourcePos1 + 1;
            
            if (sourcePos2 >= sourceSamplesToRead)
                break;  // Evitar acesso fora dos limites
                
            float fraction = static_cast<float>(sourcePos - sourcePos1);
            outputBuffer[i] = sourceBuffer[sourcePos1] + fraction * (sourceBuffer[sourcePos2] - sourceBuffer[sourcePos1]);
        }
        
        // Atualizar a posição considerando a taxa de amostragem
        position += static_cast<juce::int64>(numSamples * ratio);
        
        // Verificar se chegamos ao final do arquivo
        if (position >= length)
        {
            position = 0;
            std::cout << "End of file reached, resetting position to start." << std::endl;
        }
        
        return numSamples;
    }
    else
    {
        // Se não precisa de resampling, usar o código original
        int samplesAvailable = static_cast<int>(length - position);
        int samplesToRead = juce::jmin(numSamples, samplesAvailable);
        
        // Código original para o caso sem resampling...
        if (numChannels > 1)
        {
            for (int i = 0; i < samplesToRead; ++i)
            {
                float sample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    sample += audioData.getSample(ch, static_cast<int>(position) + i);
                }
                outputBuffer[i] = sample / static_cast<float>(numChannels);
            }
        }
        else if (numChannels == 1)
        {
            for (int i = 0; i < samplesToRead; ++i)
            {
                outputBuffer[i] = audioData.getSample(0, static_cast<int>(position) + i);
            }
        }
        
        position += samplesToRead;
        
        if (position >= length)
        {
            position = 0;
            std::cout << "End of file reached, resetting position to start." << std::endl;
        }
        
        return samplesToRead;
    }
}

double AudioFileReader::getSampleRate() const
{
    return sampleRate;
}

void AudioFileReader::resetPosition()
{
    position = 0;
}