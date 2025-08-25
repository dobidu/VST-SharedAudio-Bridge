#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include <windows.h>


#define NUM_BUFFER_CHANNELS 8
#define MAX_BUFFER_SIZE 4096

class MemoryManager {
private:
    int m_maxMemSize = MAX_BUFFER_SIZE * NUM_BUFFER_CHANNELS * sizeof(float);
    int m_maxNumOfSamples = MAX_BUFFER_SIZE * NUM_BUFFER_CHANNELS;
    float* m_shmMemAddress = nullptr;
    HANDLE m_mapFileHandle = nullptr;

public:
    MemoryManager();
    void initializeMemoryBlock();
    void fillWithZero(int bufferSize);
    void fillShmWithBuffer(const float* sourceBuffer, int sourceSize);
    void copyShmToBuffer(float* destBuffer, int destSize);
    void openMemoryBlockForWriting();
    bool openMemoryBlockForReading();
    void closeMemoryBlock();
};

#endif //MEMORYMANAGER_H
