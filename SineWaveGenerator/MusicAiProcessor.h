#ifndef MUSICAI_PROCESSOR_H
#define MUSICAI_PROCESSOR_H

#include <musicai/musicai.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <string>

class MusicAIProcessor {
public:
    // Construtor e destrutor
    MusicAIProcessor();
    ~MusicAIProcessor();

    // Constantes
    static const int EXPECTED_INPUT_CHANNELS = 2;
    static const int OUTPUT_AUDIO_CHANNELS = 8; // 4 stems 2 channels each
    static const int DEFAULT_INPUT_SIZE = 8192;
    static const int DEFAULT_OUTPUT_SIZE = 8192;

    // Método principal para processar um arquivo
    bool processFile(
        const std::string& modelPath,
        const std::string& inputFilePath,
        const std::string& outputFilePath = "out.wav");

    // Métodos auxiliares
    static void copy_soundfile_info(SF_INFO* to, const SF_INFO* from);
    static bool valid_soudfile_info(SF_INFO* info);

private:
    // Variáveis de estado ou métodos auxiliares privados podem ser adicionados aqui
};

#endif