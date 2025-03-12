#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include "JuceHeader.h"
#include "SharedMemoryManager.h"

class SineWaveGenerator
{
public:
    SineWaveGenerator() : frequency(440.0f), isRunning(false), sharedMemory()
    {
        // Inicializar o gerenciador de memória compartilhada
        if (!sharedMemory.initialize())
        {
            std::cerr << "Falha ao inicializar a memoria compartilhada" << std::endl;
            return;
        }
        
        std::cout << "Memoria compartilhada inicializada com sucesso" << std::endl;
    }
    
    ~SineWaveGenerator()
    {
        stop();
        sharedMemory.setGeneratorActive(false);
    }
    
    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        
        // Atualizar a frequência na memória compartilhada
        if (sharedMemory.isInitialized()) {
            sharedMemory.setFrequency(newFrequency);
        }
        
        std::cout << "Frequencia ajustada para " << frequency << " Hz" << std::endl;
    }
    
    void start()
    {
        if (isRunning.load())
            return;
        
        isRunning.store(true);
        
        // Indicar que o gerador está ativo
        sharedMemory.setGeneratorActive(true);
        
        generatorThread = std::thread(&SineWaveGenerator::run, this);
        
        std::cout << "Gerador de senoide iniciado" << std::endl;
    }
    
    void stop()
    {
        if (!isRunning.load())
            return;
        
        isRunning.store(false);
        
        // Indicar que o gerador está inativo
        sharedMemory.setGeneratorActive(false);
        
        if (generatorThread.joinable())
            generatorThread.join();
        
        std::cout << "Gerador de senoide parado" << std::endl;
    }
    
private:

    void run()
    {
        // Configurações
        const int bufferSize = 1024;        // Tamanho do buffer de áudio
        float phase = 0.0f;                 // Fase atual da senóide
        
        std::vector<float> buffer(bufferSize);
        
        // Manter histórico da fase entre chamadas para continuidade
        float continuousPhase = 0.0f;
        
        // Timestamp para controle de fluxo
        auto lastBufferTime = std::chrono::high_resolution_clock::now();
        
        while (isRunning.load())
        {
            // Receber taxa de amostragem atualizada do plugin
            double currentSampleRate = sharedMemory.getSampleRate();
            
            // Verificar se a taxa de amostragem é válida
            if (currentSampleRate <= 0)
                currentSampleRate = 44100.0;  // Usar valor padrão se inválido
            
            // Verificar se a frequência foi alterada
            float currentFrequency = frequency;
            
            // Calcular o tempo ideal entre buffers (em ms) para evitar gaps
            // Queremos enviar um novo buffer antes que o atual seja completamente consumido
            double bufferDurationMs = (bufferSize * 1000.0) / currentSampleRate;
            double targetRefreshMs = bufferDurationMs * 0.5; // Meio buffer de margem
            
            // Gerar senóide com fase contínua
            phase = continuousPhase; // Usar a fase continuada da iteração anterior
            
            for (int i = 0; i < bufferSize; ++i)
            {
                buffer[i] = std::sin(phase);
                
                // Incrementar a fase
                phase += 2.0f * float(juce::MathConstants<double>::pi) * currentFrequency / static_cast<float>(currentSampleRate);
                
                // Manter a fase entre 0 e 2π
                while (phase >= 2.0f * float(juce::MathConstants<double>::pi))
                    phase -= 2.0f * float(juce::MathConstants<double>::pi);
            }
            
            // Guardar a fase final para a próxima iteração
            continuousPhase = phase;
            
            // Tentativas de escrita com backoff exponencial
            bool written = false;
            int attempts = 0;
            const int maxAttempts = 10;
            
            while (!written && attempts < maxAttempts) {
                written = sharedMemory.writeAudioData(buffer.data(), bufferSize);
                
                if (!written) {
                    // Espera breve com backoff exponencial
                    std::this_thread::sleep_for(std::chrono::milliseconds(1 << attempts));
                    attempts++;
                }
            }
            
            // Calcular quanto tempo passou desde o último envio
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastBufferTime).count();
            
            // Esperar o tempo necessário para manter o fluxo constante
            // mas sem exceder o tempo máximo necessário para evitar lacunas
            if (elapsed < targetRefreshMs) {
                std::this_thread::sleep_for(std::chrono::milliseconds(
                    static_cast<int>(targetRefreshMs - elapsed)));
            }
            
            // Atualizar timestamp para o próximo ciclo
            lastBufferTime = std::chrono::high_resolution_clock::now();
        }
    }
    
    float frequency;                    // Frequência da senóide em Hz
    std::atomic<bool> isRunning;        // Flag para controlar o loop do gerador
    std::thread generatorThread;        // Thread para geração da senóide
    SharedMemoryManager sharedMemory;   // Gerenciador de memória compartilhada
};

int main(int argc, char* argv[])
{
    std::cout << "Aplicacao Geradora de Senoide para o Plugin VST de Baixa Latencia" << std::endl;
    std::cout << "=====================================================================" << std::endl;
    
    SineWaveGenerator generator;
    
    // Menu interativo
    bool quit = false;
    while (!quit)
    {
        std::cout << "\nComandos disponiveis:" << std::endl;
        std::cout << "1. Iniciar geracao" << std::endl;
        std::cout << "2. Parar geracao" << std::endl;
        std::cout << "3. Definir frequencia" << std::endl;
        std::cout << "4. Sair" << std::endl;
        
        std::cout << "\nDigite um comando: ";
        
        int command;
        std::cin >> command;
        
        switch (command)
        {
            case 1: // Iniciar
                generator.start();
                break;
                
            case 2: // Parar
                generator.stop();
                break;
                
            case 3: // Definir frequência
            {
                float newFrequency;
                std::cout << "Digite a nova frequencia (Hz): ";
                std::cin >> newFrequency;
                
                if (newFrequency > 0 && newFrequency < 20000)
                {
                    generator.setFrequency(newFrequency);
                }
                else
                {
                    std::cout << "Frequencia invalida. Deve estar entre 0 e 20000 Hz." << std::endl;
                }
                break;
            }
                
            case 4: // Sair
                generator.stop();
                quit = true;
                break;
                
            default:
                std::cout << "Comando invalido" << std::endl;
                break;
        }
    }
    
    return 0;
}