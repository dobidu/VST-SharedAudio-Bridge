#include "MemoryManager.h"
// #include <juce_audio_basics/buffers/juce_FloatVectorOperations.h>

#include <stdexcept>


MemoryManager::MemoryManager()
{
}

void MemoryManager::initializeMemoryBlock()
{
    for (int i = 0; i < m_maxNumOfSamples; i++)
        m_shmMemAddress[i] = 0;
    // juce::FloatVectorOperations::clear(m_ShmMemAddress, m_maxMemSize);
}

void MemoryManager::fillWithZero(int bufferSize)
{
    for (int i = 0; i < bufferSize; i++)
        m_shmMemAddress[i] = 0;
    // juce::FloatVectorOperations::clear(m_ShmMemAddress, m_maxMemSize);
}

void MemoryManager::fillShmWithBuffer(const float* sourceBuffer, int sourceSize)
{
    CopyMemory(m_shmMemAddress, sourceBuffer, sourceSize * sizeof(float));
    // juce::FloatVectorOperations::copy(m_ShmMemAddress, &sourceBuffer, sourceSize);
}

void MemoryManager::copyShmToBuffer(float* destBuffer, int destSize)
{
    CopyMemory(destBuffer, m_shmMemAddress, destSize * sizeof(float));
    // juce::FloatVectorOperations::copy(&destBuffer, m_ShmMemAddress, destSize);
}

void MemoryManager::openMemoryBlockForWriting()
{
    m_mapFileHandle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        m_maxMemSize,
        "Moises//SharedMemoryProc"
    );
    if (m_mapFileHandle == nullptr) {
        throw std::runtime_error("Failed to create file handle for writing.");
    }

    m_shmMemAddress = static_cast<float*>(MapViewOfFile(
        m_mapFileHandle,
        FILE_MAP_WRITE,
        0,
        0,
        m_maxMemSize)
    );
    if (m_shmMemAddress == nullptr) {
        throw std::runtime_error("Failed to map view of writer file handle");
    }
}

bool MemoryManager::openMemoryBlockForReading()
{
    HANDLE hMapFile = OpenFileMapping(
            FILE_MAP_READ,
            false,
            "Moises//SharedMemoryProc"
        );
    if (hMapFile == nullptr) {
        return false;
    }

    m_shmMemAddress = static_cast<float*>(MapViewOfFile(
        hMapFile,
        FILE_MAP_READ,
        0,
        0,
        m_maxMemSize)
    );
    if (m_shmMemAddress == nullptr) {
        return false;
    }

    return true;
}

void MemoryManager::closeMemoryBlock()
{
    UnmapViewOfFile(m_shmMemAddress);
    CloseHandle(m_mapFileHandle);
}