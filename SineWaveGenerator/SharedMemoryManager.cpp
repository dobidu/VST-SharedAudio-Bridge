#include "SharedMemoryManager.h"

#if JUCE_WINDOWS
    #include <windows.h>
#elif JUCE_LINUX
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#elif JUCE_MAC
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#endif

// Implementação da classe PlatformSharedMemory
SharedMemoryManager::PlatformSharedMemory::PlatformSharedMemory(const std::string& name, size_t size)
    : memoryName(name), memSize(size), data(nullptr), isCreated(false), isOwner(false)
{
#if JUCE_WINDOWS
    // Tentar abrir memória compartilhada existente
    fileHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    
    if (fileHandle == nullptr)
    {
        // Criar nova memória compartilhada
        fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 
                                       static_cast<DWORD>(size), name.c_str());
        isOwner = (fileHandle != nullptr);
    }
    
    if (fileHandle != nullptr)
    {
        // Mapear a memória compartilhada para o espaço de endereço do processo
        data = MapViewOfFile(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
        isCreated = (data != nullptr);
        
        if (isOwner && isCreated)
        {
            // Se fomos nós que criamos, inicializar a memória
            ZeroMemory(data, size);
        }
    }
#elif JUCE_MAC || JUCE_LINUX
    // Em sistemas POSIX (Mac, Linux), usar mmap
    std::string fullName = "/" + name; // Adicionar slash para caminho absoluto
    
    // Tentar abrir memória compartilhada existente
    fileDescriptor = shm_open(fullName.c_str(), O_RDWR, 0666);
    
    if (fileDescriptor == -1)
    {
        // Criar nova memória compartilhada
        fileDescriptor = shm_open(fullName.c_str(), O_CREAT | O_RDWR, 0666);
        isOwner = (fileDescriptor != -1);
        
        if (isOwner)
        {
            // Definir o tamanho da memória compartilhada
            if (ftruncate(fileDescriptor, static_cast<off_t>(size)) == -1)
            {
                close(fileDescriptor);
                fileDescriptor = -1;
                isOwner = false;
            }
        }
    }
    
    if (fileDescriptor != -1)
    {
        // Mapear a memória compartilhada
        data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
        
        if (data == MAP_FAILED)
        {
            data = nullptr;
        }
        
        isCreated = (data != nullptr);
        
        if (isOwner && isCreated)
        {
            // Se fomos nós que criamos, inicializar a memória
            memset(data, 0, size);
        }
    }
#endif
}

SharedMemoryManager::PlatformSharedMemory::~PlatformSharedMemory()
{
#if JUCE_WINDOWS
    if (data != nullptr)
    {
        UnmapViewOfFile(data);
    }
    
    if (fileHandle != nullptr)
    {
        CloseHandle(fileHandle);
    }
#elif JUCE_MAC || JUCE_LINUX
    if (data != nullptr && data != MAP_FAILED)
    {
        munmap(data, memSize);
    }
    
    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
        
        // Se somos o dono, remover o objeto de memória compartilhada
        if (isOwner)
        {
            std::string fullName = "/" + memoryName;
            shm_unlink(fullName.c_str());
        }
    }
#endif
}

// Implementação da classe SharedMemoryManager
SharedMemoryManager::SharedMemoryManager()
    : sharedData(nullptr), initialized(false)
{
}

SharedMemoryManager::~SharedMemoryManager()
{
    sharedMemoryBlock.reset();
}

bool SharedMemoryManager::initialize()
{
    std::unique_lock<std::mutex> lock(accessMutex);
    
    // Criar/abrir memória compartilhada
    sharedMemoryBlock = std::make_unique<PlatformSharedMemory>(sharedMemoryName, sharedMemorySize);
    
    if (!sharedMemoryBlock->isValid())
    {
        return false;
    }
    
    // Obter ponteiro para os dados
    sharedData = static_cast<AudioSharedData*>(sharedMemoryBlock->getData());
    
    if (sharedData == nullptr)
    {
        sharedMemoryBlock.reset();
        return false;
    }
    
    initialized = true;
    return true;
}

bool SharedMemoryManager::readAudioData(juce::AudioBuffer<float>& buffer, int numSamples, float& latencyMs)
{
    if (!initialized || sharedData == nullptr)
        return false;
    
    std::unique_lock<std::mutex> lock(accessMutex);
    
    // Verificar se há dados disponíveis
    if (!sharedData->dataReady.load())
        return false;
    
    const int bufferSize = sharedData->bufferSize.load();
    if (bufferSize <= 0)
        return false;
    
    // Calcular latência
    uint64_t timestampNow = std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    uint64_t timestampOld = sharedData->timestamp.load();
    latencyMs = static_cast<float>(timestampNow - timestampOld) / 1000.0f;
    
    // Copiar dados para o buffer de áudio
    int readPos = sharedData->readPosition.load();
    
    const int samplesToRead = juce::jmin(numSamples, bufferSize);
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < samplesToRead; ++i)
        {
            const int index = (readPos + i) % AudioSharedData::maxBufferSize;
            channelData[i] = sharedData->audioData[index];
        }
    }
    
    // Atualizar posição de leitura
    readPos = (readPos + samplesToRead) % AudioSharedData::maxBufferSize;
    sharedData->readPosition.store(readPos);
    
    //
    if (samplesToRead >= bufferSize) {
        sharedData->dataReady.store(false);
    } else {
        sharedData->bufferSize.store(bufferSize - samplesToRead);
    }
    
    return true;
}

bool SharedMemoryManager::writeAudioData(const float* data, int numSamples)
{
    if (!initialized || sharedData == nullptr)
        return false;
    
    std::unique_lock<std::mutex> lock(accessMutex);
    
    // Verificar se o buffer anterior já foi consumido antes de escrever novo dado
    if (sharedData->dataReady.load()) {
        // Se ainda há dados não processados, aguardar
        return false;
    }
    
    // Limitar ao tamanho máximo do buffer
    const int samplesToWrite = juce::jmin(numSamples, AudioSharedData::maxBufferSize);

    // Guardar a taxa de amostragem original
    sharedData->originalSampleRate.store(sharedData->sampleRate.load());
    
    // Copiar os dados
    for (int i = 0; i < samplesToWrite; ++i) {
        sharedData->audioData[i] = data[i];
    }
    
    // Registrar timestamp para medição de latência
    sharedData->timestamp.store(std::chrono::duration_cast<std::chrono::microseconds>(
                               std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    
    // Atualizar posição de escrita e tamanho do buffer
    sharedData->writePosition.store(0); // Sempre começa do zero
    sharedData->bufferSize.store(samplesToWrite);
    sharedData->readPosition.store(0); // Reset da posição de leitura
    
    // Marcar dados como prontos para leitura
    sharedData->dataReady.store(true);
    
    return true;
}

void SharedMemoryManager::setSampleRate(double newSampleRate)
{
    if (initialized && sharedData != nullptr)
    {
        std::unique_lock<std::mutex> lock(accessMutex);
        sharedData->sampleRate.store(newSampleRate);
    }
}

double SharedMemoryManager::getSampleRate() const
{
    if (initialized && sharedData != nullptr)
    {
        return sharedData->sampleRate.load();
    }
    
    return 44100.0; // valor padrão
}