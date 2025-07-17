#include "MemoryManager.h"
// #include <juce_audio_basics/buffers/juce_FloatVectorOperations.h>

#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

MemoryManager::MemoryManager()
{
    key_t key = ftok("moises_mmapf", 'R');
    m_Shmid = shmget(key, sizeof(float) * m_maxMemSize, 0644 | IPC_CREAT);
    m_ShmMemAddress = nullptr;
}

void MemoryManager::clearShm()
{
    shmctl(m_Shmid, IPC_RMID, nullptr);
}

void MemoryManager::fetchShmPointer()
{
    m_ShmMemAddress = static_cast<float*>(shmat(m_Shmid, nullptr, 0));
}

int MemoryManager::detachShmPointer()
{
    return shmdt(m_ShmMemAddress);
}

void MemoryManager::initializeMemoryBlock()
{
    fetchShmPointer();
    for (int i = 0; i < m_maxMemSize; i++)
        m_ShmMemAddress[i] = 0;
    // juce::FloatVectorOperations::clear(m_ShmMemAddress, m_maxMemSize);
    detachShmPointer();
}

void MemoryManager::fillShmWithBuffer(const float* sourceBuffer, int sourceSize)
{
    fetchShmPointer();
    std::memcpy(m_ShmMemAddress, sourceBuffer, sizeof(float) * sourceSize);
    // juce::FloatVectorOperations::copy(m_ShmMemAddress, &sourceBuffer, sourceSize);
    detachShmPointer();
}

void MemoryManager::fillWithZero(int bufferSize)
{
    fetchShmPointer();
    for (int i = 0; i < bufferSize; i++)
        m_ShmMemAddress[i] = 0;
    detachShmPointer();
}

void MemoryManager::copyShmToBuffer(float* destBuffer, int destSize)
{
    fetchShmPointer();
    std::memcpy(destBuffer, m_ShmMemAddress, sizeof(float) * destSize);
    // juce::FloatVectorOperations::copy(&destBuffer, m_ShmMemAddress, destSize);
    detachShmPointer();
}

