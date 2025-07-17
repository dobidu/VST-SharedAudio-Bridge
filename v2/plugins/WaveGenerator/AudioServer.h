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
    int playhead{0};
    int numOfSamples;
    juce::AudioBuffer<float> buffer;
    ZMQCommServer server;
    MemoryManager memoryManager;
    void updatePlayhead();

public:
    AudioServer();
    void run() override;

    juce::AudioBuffer<float>& getBuffer();
    void setNumberOfSamples(int num);
    void startServer();
    void stopServer();
};



#endif //AUDIOSERVER_H
