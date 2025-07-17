#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

class MemoryManager {
private:
    int m_Shmid;
    int m_maxMemSize = 4096;
    float* m_ShmMemAddress;

public:
    MemoryManager();
    void clearShm();
    void initializeMemoryBlock();
    void fillShmWithBuffer(const float* sourceBuffer, int sourceSize);
    void copyShmToBuffer(float* destBuffer, int destSize);
    void fillWithZero(int bufferSize);

private:
    void fetchShmPointer();
    int detachShmPointer();
};

#endif //MEMORYMANAGER_H
