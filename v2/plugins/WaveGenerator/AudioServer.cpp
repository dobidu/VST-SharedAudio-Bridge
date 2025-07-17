//
// Created by gferraz on 7/17/25.
//

#include "AudioServer.h"

AudioServer::AudioServer() : juce::Thread("AudioServer"), server(), memoryManager()
{
}

juce::AudioBuffer<float>& AudioServer::getBuffer()
{
    return buffer;
}

void AudioServer::setNumberOfSamples(int num)
{
    numOfSamples = num;
}

void AudioServer::run()
{
    int bufferSize = 1;
    while (bufferSize > 0)
    {
        bufferSize = server.receiveBufferRequest();
        DBG("Playhead = " << playhead << " BufferSize = " << bufferSize << " SampleSize = " << numOfSamples);
        if (playhead + bufferSize < numOfSamples)
        {
            memoryManager.fillShmWithBuffer(buffer.getReadPointer(0) + playhead, bufferSize);
            playhead += bufferSize;
        }
        else
        {
            memoryManager.fillWithZero(bufferSize);
            playhead = 0;
        }
        server.sendBufferReadyResponse();
    }
}

void AudioServer::startServer()
{
    memoryManager.initializeMemoryBlock();
    startThread(juce::Thread::Priority::highest);
}

void AudioServer::stopServer()
{
    stopThread(1000);
}
