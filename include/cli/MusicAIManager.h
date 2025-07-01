#ifndef CLI_MUSICAI_MANAGER_H
#define CLI_MUSICAI_MANAGER_H

#include <musicai/musicai.h>
#include <sndfile.h>
#include <string>
#include <vector>
#include <memory>

namespace cli {

    /**
     * @brief Class responsible for managing the MusicAI library context and providing high-level methods
     *        for CLI commands such as process, batch-process, audio-info, model-info, and check-model.
     */
    class MusicAIManager {
    public:
        // Constantes
        static const int EXPECTED_INPUT_CHANNELS = 2;
        static const int OUTPUT_AUDIO_CHANNELS = 8; // 4 stems 2 channels each
        static const int DEFAULT_INPUT_SIZE = 8192;
        static const int DEFAULT_OUTPUT_SIZE = 8192;

        /**
         * @brief Returns the singleton instance.
         */
        static MusicAIManager& getInstance();

        /**
         * @brief Processes an audio file using a specific model and outputs the result.
         * @param modelPath Path to the model file.
         * @param inputPath Path to the input audio file.
         * @param outputPath Path for the output audio file.
         * @param configPath (Optional) Path to a processor configuration file.
         * @return true if processing is successful, false otherwise.
         */
        bool process(const std::string& modelPath,
            const std::string& inputPath,
            const std::string& outputPath,
            const std::string& configPath = "");

        /**
         * @brief Processes all audio files in a directory using one or more model files.
         * @param inputDirectory Path to the directory containing input audio files.
         * @param modelPaths Vector with the paths to model files.
         * @param outputDirectory Optional: Where to place output files (default: ./results).
         * @param configPath Optional: Path to configuration file (applies to all).
         * @return true if all files were processed without errors.
         */
        bool batchProcess(const std::vector<std::string>& inputFiles,
            const std::vector<std::string>& modelPaths,
            const std::string& outputDirectory,
            const std::string& configPath);

        /**
         * @brief Retrieve and display information about a model file (does not fully load the model).
         * @param modelPath Path to the model file.
         * @param info Informations about the model.
         * @return true if info could be retrieved, false otherwise.
         */
        bool modelInfo(const std::string& modelPath, musicai_model_info& info);

        /**
         * @brief Checks if a model file can be successfully loaded by the library.
         * @param modelPath Path to the model file.
         * @param info Informations about the model.
         * @return true if model initializes correctly, false otherwise.
         */
        bool checkModel(const std::string& modelPath);

        /**
         * @brief Returns true if the library context has been created.
         */
        bool isInitialized() const;

        /**
         * @brief (Usually for internal/test use) Creates the library context if it does not exist.
         * @return true if context is ready to use.
         */
        bool create();

        /**
         * @brief Destroys the current library context (frees memory/resources).
         */
        void destroy();

        /**
         * @brief (Usually for internal/test use) Destroys and re-creates the context.
         * @return true if context was successfully reset.
         */
        bool reset();

        /**
         * @brief Returns the raw context pointer (use with caution).
         */
        musicai* get();

        void copy_soundfile_info(SF_INFO* to, const SF_INFO* from);

        bool valid_soudfile_info(SF_INFO* info);

    private:
        MusicAIManager() = default;
        ~MusicAIManager();

        // Prevent copy and move
        MusicAIManager(const MusicAIManager&) = delete;
        MusicAIManager& operator=(const MusicAIManager&) = delete;

        musicai* context_ = nullptr;
    };

} // namespace cli

#endif // CLI_MUSICAI_MANAGER_H