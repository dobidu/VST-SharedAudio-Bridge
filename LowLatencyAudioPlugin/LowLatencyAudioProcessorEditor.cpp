#include "LowLatencyAudioProcessorEditor.h"
#include "LowLatencyAudioPlugin.h" // Incluir o processador aqui

//==============================================================================
LowLatencyAudioProcessorEditor::LowLatencyAudioProcessorEditor (LowLatencyAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Configurar botão de reprodução
    playButton.setButtonText("Play");
    playButton.onClick = [this]() { 
        audioProcessor.togglePlayback(); 
        playButton.setButtonText(audioProcessor.isPlaying() ? "Stop" : "Play");
    };
    addAndMakeVisible(playButton);
    
    // Configurar label de latência
    latencyLabel.setText("Latencia:", juce::dontSendNotification);
    latencyLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(latencyLabel);
    
    // Configurar label de valor de latência
    latencyValueLabel.setText("0.0 ms", juce::dontSendNotification);
    latencyValueLabel.setFont(juce::Font(14.0f));
    latencyValueLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(latencyValueLabel);

    // Configurar label de frequência
    frequencyLabel.setText("Frequencia:", juce::dontSendNotification);
    frequencyLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(frequencyLabel);
    
    // Configurar label de valor de frequência
    frequencyValueLabel.setText("440.0 Hz", juce::dontSendNotification);
    frequencyValueLabel.setFont(juce::Font(14.0f));
    frequencyValueLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(frequencyValueLabel);

    // Configurar label de status do gerador
    statusLabel.setText("Status:", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(statusLabel);
    
    // Configurar label de valor do status
    statusValueLabel.setText("Desconectado", juce::dontSendNotification);
    statusValueLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    statusValueLabel.setJustificationType(juce::Justification::right);
    statusValueLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    addAndMakeVisible(statusValueLabel);
    
    // Iniciar timer para atualização da interface
    startTimer(50); // Atualizar a cada 50 ms
    
    // Tamanho da janela do plugin
    setSize (400, 200);
}

LowLatencyAudioProcessorEditor::~LowLatencyAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void LowLatencyAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Preencher o fundo
    g.fillAll (juce::Colours::darkgrey);
    
    // Desenhar título
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Low Latency Audio Plugin", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void LowLatencyAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50); // Espaço para o título
    
    // Layout do botão de reprodução
    auto buttonArea = area.removeFromTop(60);
    playButton.setBounds(buttonArea.reduced(80, 10));

    // Layout dos labels de status
    auto statusArea = area.removeFromTop(40);
    statusLabel.setBounds(statusArea.removeFromLeft(200).reduced(20, 5));
    statusValueLabel.setBounds(statusArea.reduced(20, 5));
    
    // Layout dos labels de latência
    auto latencyArea = area.removeFromTop(40);
    latencyLabel.setBounds(latencyArea.removeFromLeft(200).reduced(20, 5));
    latencyValueLabel.setBounds(latencyArea.reduced(20, 5));
    
    // Layout dos labels de frequência
    auto freqArea = area.removeFromTop(40);
    frequencyLabel.setBounds(freqArea.removeFromLeft(200).reduced(20, 5));
    frequencyValueLabel.setBounds(freqArea.reduced(20, 5));
}

void LowLatencyAudioProcessorEditor::timerCallback()
{
    // Atualizar a exibição de latência com valor filtrado para evitar oscilações extremas
    float latency = audioProcessor.getCurrentLatency();
    
    // Filtrar valores absurdos (acima de 1000ms provavelmente são erros)
    if (latency > 0.0f && latency < 1000.0f) {
        // Aplicar filtro de média móvel para suavizar a exibição
        static float latencyHistory[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        static int historyIndex = 0;
        
        latencyHistory[historyIndex] = latency;
        historyIndex = (historyIndex + 1) % 5;
        
        float averageLatency = 0.0f;
        for (int i = 0; i < 5; ++i) {
            averageLatency += latencyHistory[i];
        }
        averageLatency /= 5.0f;
        
        // Mostrar com apenas 1 casa decimal para melhor legibilidade
        latencyValueLabel.setText(juce::String(averageLatency, 1) + " ms", juce::dontSendNotification);
    } else {
        latencyValueLabel.setText("--.- ms", juce::dontSendNotification);
    }

    // Atualizar o status do gerador
    bool generatorActive = audioProcessor.isGeneratorActive();
    if (generatorActive) {
        statusValueLabel.setText("Conectado", juce::dontSendNotification);
        statusValueLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    } else {
        statusValueLabel.setText("Desconectado", juce::dontSendNotification);
        statusValueLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }

    // Atualizar a exibição de frequência
    float freq = audioProcessor.getCurrentFrequency();
    frequencyValueLabel.setText(juce::String(freq, 1) + " Hz", juce::dontSendNotification);
    // Atualizar texto do botão
    playButton.setButtonText(audioProcessor.isPlaying() ? "Stop" : "Play");
}

