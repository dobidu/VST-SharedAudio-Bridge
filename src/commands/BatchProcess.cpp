#include "cli/MusicAIManager.h"
#include "cli/commands/BatchProcess.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>

namespace cli {

    // Utilitário para dividir string separada por vírgula em lista de modelos
    static std::vector<std::string> split_comma_separated(const std::string& s) {
        std::vector<std::string> out;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty()) out.push_back(item);
        }
        return out;
    }

    // Exibe uso
    static void print_batch_process_usage() {
        std::cout << "Usage: cli-musicai batch-process "
            "--input <file or dir> "
            "--models <model1,onxx,model2.onnx,...> "
            "[--output <output_dir>]\n"
            "  --input <file/dir>    Audio file or directory with .wav files (required)\n"
            "  --models <list>       Comma-separated list of model files (required)\n"
            "  --output <dir>        Output directory (optional, default: ./results)\n"
            "  --help                Show this help message\n";
    }

    int cmd_batch_process(int argc, char** argv) {
        // Procura por --help
        for (int i = 1; i < argc; ++i)
            if (std::string(argv[i]) == "--help") {
                print_batch_process_usage();
                return 0;
            }

        std::string inputArg, modelListArg, outputDir = "results";
        // Parse argumentos
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--input" && i + 1 < argc) {
                inputArg = argv[++i];
            }
            else if (std::string(argv[i]) == "--models" && i + 1 < argc) {
                modelListArg = argv[++i];
            }
            else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
                outputDir = argv[++i];
            }
        }

        if (inputArg.empty() || modelListArg.empty()) {
            std::cerr << "[BatchProcess] Error: --input and --models are required!\n";
            print_batch_process_usage();
            return 1;
        }

        std::vector<std::string> modelFiles = split_comma_separated(modelListArg);

        // Determina se inputArg é diretório ou arquivo único
        namespace fs = std::filesystem;
        std::vector<std::string> inputFiles;

        if (fs::is_directory(inputArg)) {
            // Lista todos arquivos .wav
            for (const auto& entry : fs::directory_iterator(inputArg)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".wav" || ext == ".WAV")
                        inputFiles.push_back(entry.path().string());
                }
            }
            if (inputFiles.empty()) {
                std::cerr << "[BatchProcess] No .wav files found in directory: " << inputArg << std::endl;
                return 1;
            }
        }
        else if (fs::is_regular_file(inputArg)) {
            inputFiles.push_back(inputArg);
        }
        else {
            std::cerr << "[BatchProcess] --input is not a file or directory: " << inputArg << std::endl;
            return 1;
        }

        // Chama o manager
        if (!MusicAIManager::getInstance().batchProcess(inputFiles, modelFiles, outputDir, "")) {
            std::cerr << "[BatchProcess] Batch processing failed.\n";
            return 1;
        }

        std::cout << "[BatchProcess] Batch processing completed successfully.\n";
        return 0;
    }

} // namespace cli