#include <iostream>
#include <string>
#include "cli/commands/Process.h"
#include "cli/commands/BatchProcess.h"
#include "cli/commands/ModelInfo.h"
#include "cli/commands/CheckModel.h"

void print_help() {
    std::cout << R"(
============================================
              MusicAI - Command Line
============================================
Usage:
  cli-musicai <command> [options]
Available commands:
  process --model <model> --input <input> --output <output> [--config <config>]
      Processes the <input> audio file using <model> and saves the result to <output>.
      Parameters:
        <model>    Path to the model file (e.g., model.onnx)
        <input>    Path to the input audio file (e.g., input.wav)
        <output>   Path to the output audio file (e.g., output.wav)
        [<config>] Path to a JSON processor configuration file (optional)
  batch-process --input <directory> --models <models> [--output <directory>]
      Processes all audio files in <directory> using a list of models.
      Parameters:
        <directory>  Directory containing input audio files
        <models>   Comma-separated list of model files (e.g., model1.mai,model2.mai)
        [<directory>] Output directory for processed files (default: ./results)
  model-info --model <file>
      Displays information about the specified model file without loading it.
  check-model --model <file>
      Verifies if the given model file is valid and can be loaded.
  help
      Shows this help message.

For more information on a specific command, use:
  cli-musicai <command> --help
============================================
)" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string command = argv[1];

    if (command == "help" || command == "--help") {
        print_help();
        return 0;
    }

    if (command == "process") {
        return cli::cmd_process(argc - 1, argv + 1);
    }
    if (command == "batch-process") {
        return cli::cmd_batch_process(argc - 1, argv + 1);
    }
    if (command == "model-info") {
        return cli::cmd_model_info(argc - 1, argv + 1);
    }
    if (command == "check-model") {
        return cli::cmd_check_model(argc - 1, argv + 1);
    }

    std::cerr << "Unknown command: " << command << std::endl;
    print_help();
    return 1;
}