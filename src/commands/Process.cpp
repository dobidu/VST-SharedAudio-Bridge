#include "cli/MusicAIManager.h"
#include "cli/commands/Process.h"
#include <cstring>
#include <iostream>

namespace cli {

    static void print_usage() {
        std::cout << "Usage: cli-musicai process --model <model> --input <input> --output <output> [--config <config>]\n";
        std::cout << "Options:\n"
            "  --help              Show this help message\n"
            "  --model <model>     Path to model file (required)\n"
            "  --input <input>     Path to input audio file (required)\n"
            "  --output <output>   Path to output audio file (required)\n"
            "  --config <config>   Path to config file (optional)\n";
    }

    int cmd_process(int argc, char** argv) {
        // Defaults
        std::string modelPath, inputPath, outputPath, configPath;

        // Argument parsing
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "--help") == 0) {
                print_usage();
                return 0;
            }
            else if (std::strcmp(argv[i], "--model") == 0 && i + 1 < argc) {
                modelPath = argv[++i];
            }
            else if (std::strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
                inputPath = argv[++i];
            }
            else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
                outputPath = argv[++i];
            }
            else if (std::strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
                configPath = argv[++i];
            }
            else {
                std::cerr << "Unknown or incomplete argument: " << argv[i] << std::endl;
                print_usage();
                return 1;
            }
        }

        // Validate required parameters
        if (modelPath.empty() || inputPath.empty() || outputPath.empty()) {
            std::cerr << "Missing required parameters!\n";
            print_usage();
            return 1;
        }

        // Call the manager; it handles all internals (create/init/destroy).
        bool ok = MusicAIManager::getInstance().process(modelPath, inputPath, outputPath, configPath);
        if (!ok) {
            std::cerr << "Processing failed.\n";
            return 1;
        }

        std::cout << "Processing completed successfully. Output: " << outputPath << std::endl;
        return 0;
    }

} // namespace cli