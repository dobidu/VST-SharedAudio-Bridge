#include "LowLatencyAudioPlugin.h"
#include "LowLatencyAudioProcessorEditor.h" // Adicionar o include aqui

//==============================================================================
LowLatencyAudioProcessor::LowLatencyAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Inicializar o gerenciador de memória compartilhada
    if (!sharedMemory.initialize())
    {
        // Lidar com erro de inicialização
        juce::Logger::writeToLog("Falha ao inicializar a memória compartilhada");
    }
}

LowLatencyAudioProcessor::~LowLatencyAudioProcessor()
{
}

//==============================================================================
const juce::String LowLatencyAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LowLatencyAudioProcessor::acceptsMidi() const
{
    return false;
}

bool LowLatencyAudioProcessor::producesMidi() const
{
    return false;
}

bool LowLatencyAudioProcessor::isMidiEffect() const
{
    return false;
}

double LowLatencyAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LowLatencyAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LowLatencyAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LowLatencyAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String LowLatencyAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void LowLatencyAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================

void LowLatencyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Informar a taxa de amostragem para a aplicação externa
    sharedMemory.setSampleRate(sampleRate);

    // Preparar buffer de áudio
    audioBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    audioBuffer.clear();
    
    // Preparar buffer de backup
    lastBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    lastBuffer.clear();
    
    // Inicializar timestamp
    lastTimestamp = std::chrono::high_resolution_clock::now();
    lastDataReceived = lastTimestamp; // Inicializar o timestamp de dados recebidos

    // Inicializar frequência
    currentFrequency.store(sharedMemory.getFrequency());

    // Reset do detector de timeout
    timeoutDetected.store(false);
}

void LowLatencyAudioProcessor::releaseResources()
{
    // Liberar recursos quando o plugin é desativado
    playing.store(false);
}

bool LowLatencyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Suportar apenas layouts estéreo ou mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void LowLatencyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Limpar buffers não utilizados
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    // Verificar primeiro se o gerador está ativo
    if (!playing.load() || !sharedMemory.isGeneratorActive())
    {
        buffer.clear();
        hasValidData.store(false);
        return;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto timeSinceLastData = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now - lastDataReceived);
    
    // Se não recebermos dados após o timeout E o gerador não estiver ativo, parar a reprodução
    if (timeSinceLastData > dataTimeout && !sharedMemory.isGeneratorActive()) {
        timeoutDetected.store(true);
        buffer.clear();
        hasValidData.store(false);
        return;
    }    

    // Ler dados da memória compartilhada
    float latency;
    bool dataRead = sharedMemory.readAudioData(buffer, buffer.getNumSamples(), latency);
    
    if (dataRead)
    {
        // Filtrar valores de latência extremos ou negativos
        if (latency > 0.0f && latency < 1000.0f) {
            currentLatency.store(latency);
        }
        
        // Verificar se a frequência mudou
        float newFrequency = sharedMemory.getFrequency();
        float oldFrequency = currentFrequency.load();
        
        if (std::abs(newFrequency - oldFrequency) > 0.1f) {
            currentFrequency.store(newFrequency);
        }
        
        // Armazenar uma cópia do buffer para reuso em caso de falha de leitura
        if (lastBuffer.getNumChannels() != buffer.getNumChannels() || 
            lastBuffer.getNumSamples() != buffer.getNumSamples()) {
            lastBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
        }
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            lastBuffer.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
        }
        
        hasValidData.store(true);
    }
    else if (hasValidData.load())
    {
        // Se não conseguimos ler novos dados, mas temos dados anteriores válidos,
        // verificar se o gerador ainda está ativo antes de reutilizar o buffer
        
        if (!sharedMemory.isGeneratorActive()) {
            // Se o gerador parou, limpar o buffer e não reutilizar dados antigos
            buffer.clear();
            hasValidData.store(false);
        } else {
            // Caso contrário, reutilizar o último buffer
            for (int ch = 0; ch < juce::jmin(buffer.getNumChannels(), lastBuffer.getNumChannels()); ++ch) {
                buffer.copyFrom(ch, 0, lastBuffer, ch, 0, juce::jmin(buffer.getNumSamples(), lastBuffer.getNumSamples()));
            }
        }
    }
    else
    {
        // Se não temos dados válidos nem histórico, silenciar a saída
        buffer.clear();
    }
}

//==============================================================================
bool LowLatencyAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LowLatencyAudioProcessor::createEditor()
{
    return new LowLatencyAudioProcessorEditor (*this);
}

//==============================================================================
void LowLatencyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Salvar estado do plugin
    juce::MemoryOutputStream stream(destData, true);
    stream.writeFloat(currentLatency.load());
    stream.writeBool(playing.load());
}

void LowLatencyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restaurar estado do plugin
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    currentLatency.store(stream.readFloat());
    playing.store(stream.readBool());
}

void LowLatencyAudioProcessor::togglePlayback()
{
    playing.store(!playing.load());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LowLatencyAudioProcessor();
}