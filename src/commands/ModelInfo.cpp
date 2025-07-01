#include "cli/MusicAIManager.h"
#include "cli/commands/ModelInfo.h"
#include "musicai/musicai.h"
#include <iostream>

namespace cli {

    static void print_model_info_usage() {
        std::cout << "Usage: cli-musicai model-info --model <file>\n"
            << "  --model <file>    Path to the model file (.onnx)\n"
            << "  --help            Show this help message\n";
    }

    int cmd_model_info(int argc, char** argv) {
        for (int i = 1; i < argc; ++i)
            if (std::string(argv[i]) == "--help") {
                print_model_info_usage();
                return 0;
            }

        std::string modelPath;
        for (int i = 1; i < argc - 1; ++i) {
            if (std::string(argv[i]) == "--model") {
                modelPath = argv[i + 1];
                break;
            }
        }

        if (modelPath.empty()) {
            std::cerr << "[ModelInfo] Error: --model <file> is required!\n";
            print_model_info_usage();
            return 1;
        }

        musicai_model_info info;
        if (!MusicAIManager::getInstance().modelInfo(modelPath, info)) {
            std::cerr << "[ModelInfo] Failed to get model info from file: " << modelPath << std::endl;
            return 1;
        }

        // Impressão das informações do modelo
        std::cout << "==========================" << std::endl;
        std::cout << " Model Info: " << modelPath << std::endl;
        std::cout << "--------------------------" << std::endl;
        std::cout << " Input channels:    " << info.num_input_channels << std::endl;
        std::cout << " Output channels:   " << info.num_output_channels << std::endl;
        std::cout << "==========================" << std::endl;
        return 0;
    }

} // namespace cli