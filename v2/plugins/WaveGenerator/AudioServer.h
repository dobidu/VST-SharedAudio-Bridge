//
// Created by gferraz on 7/17/25.
//

#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <JuceHeader.h>
#include <MemoryManager.h>
#include "ZMQCommServer.h"

class AudioServer : juce::Thread {
private:
    bool running{false};
    int playhead{0};
    int numOfSamples{0}, numChannels{0};
    juce::Array<float> servingBuffer;
    ZMQCommServer server;
    MemoryManager memoryManager;
    void updatePlayhead();

public:
    AudioServer();
    void run() override;

    juce::Array<float>& getBuffer();
    void setBuffer(juce::Array<float>& buffer);
    void setNumberOfSamples(int num);
    void startServer();
    void stopServer();

private:
    void initAudioServer();
};

#endif //AUDIOSERVER_H
