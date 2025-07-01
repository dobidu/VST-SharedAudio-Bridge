#include "cli/MusicAIManager.h"
#include "cli/commands/ModelInfo.h"
#include "musicai/musicai.h"
#include <iostream>

namespace cli {

    static void print_check_model_usage() {
        std::cout << "Usage: cli-musicai check-model --model <file> [--config <config.json>]\n"
            "  --model <file>     Path to the model file (.mai)\n"
            "  --help             Show this help message\n";
    }

    int cmd_check_model(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--help") {
                print_check_model_usage();
                return 0;
            }
        }

        std::string modelPath;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--model" && i + 1 < argc) {
                modelPath = argv[++i];
            }
        }

        if (modelPath.empty()) {
            std::cerr << "[CheckModel] Error: --model <file> is required!\n";
            print_check_model_usage();
            return 1;
        }

        if (!MusicAIManager::getInstance().checkModel(modelPath)){
            std::cerr << "[CheckModel] Model check failed: " << modelPath << std::endl;
            return 1;
        }

        std::cout << "[CheckModel] Model check succeeded: " << modelPath << std::endl;
        return 0;
    }

}