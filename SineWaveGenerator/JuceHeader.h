#pragma once

// Incluir apenas os módulos JUCE realmente necessários
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>

// Definições específicas do projeto
#define PROJECT_NAME "SineWaveGenerator"
#define PROJECT_VERSION "1.0.0"
#define PROJECT_COMPANY_NAME "dobidu"

// Configurações JUCE
#define JUCE_STANDALONE_APPLICATION 1
#define JUCE_REPORT_APP_USAGE 0
#define JUCE_WEB_BROWSER 0
#define JUCE_USE_CURL 0
#define JUCE_APPLICATION_ENTRY_POINT 0