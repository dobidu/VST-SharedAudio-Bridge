//
// Created by gferraz on 7/17/25.
//

#include "AudioServer.h"

AudioServer::AudioServer() : juce::Thread("AudioServer"), server(), memoryManager()
{
}

juce::Array<float>& AudioServer::getBuffer()
{
    return servingBuffer;
}

void AudioServer::setBuffer(juce::Array<float>& buffer)
{
    servingBuffer = buffer;
}

void AudioServer::setNumberOfSamples(int num)
{
    numOfSamples = num;
}

void AudioServer::run()
{
    DBG("Running audio server...");
    initAudioServer();
    while (running)
    {
        int bufferSize = server.receiveBufferRequest() * 8;
        DBG("Playhead = " << playhead << " BufferSize = " << bufferSize << " SampleSize = " << numOfSamples);
        if (playhead + bufferSize < numOfSamples)
        {
            memoryManager.fillShmWithBuffer(servingBuffer.data() + playhead, bufferSize);
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
    DBG("Starting audio server...");
    memoryManager.openMemoryBlockForWriting();
    memoryManager.initializeMemoryBlock();
    startThread(juce::Thread::Priority::highest);
}

void AudioServer::stopServer()
{
    DBG("Stopping audio server...");
    running = false;
    server.closeServer();
    memoryManager.closeMemoryBlock();
    signalThreadShouldExit();
}

void AudioServer::initAudioServer() {
    running = true;
    playhead = 0;
    server.startServer();
}
