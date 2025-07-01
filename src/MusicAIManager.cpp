#include "cli/MusicAIManager.h"
#include <iostream>
#include <mutex>
#include <utility>
#include <filesystem>
namespace fs = std::filesystem;

namespace cli {

	// --- Singleton Management ---

	/**
	 * @brief Destructor. Ensures resources are properly released.
	 */
	MusicAIManager::~MusicAIManager() {
		destroy();
	}

	/**
	 * @brief Returns the singleton instance (thread-safe in C++11 and later).
	 */
	MusicAIManager& MusicAIManager::getInstance() {
		static MusicAIManager instance;
		return instance;
	}

	// --- Context Management ---

	/**
	 * @brief Creates the MusicAI context if it doesn't already exist.
	 * @return true if created or already exists, false on error
	 */
	bool MusicAIManager::create() {
		if (context_ != nullptr)
			return true; // Already created

		context_ = musicai_create();
		if (!context_) {
			std::cerr << "[MusicAIManager] Failed to create MusicAI context." << std::endl;
			return false;
		}
		return true;
	}

	/**
	 * @brief Destroys the current MusicAI context and releases resources.
	 */
	void MusicAIManager::destroy() {
		if (context_ != nullptr) {
			musicai_destroy(&context_);
			context_ = nullptr;
		}
	}

	/**
	 * @brief Resets the MusicAI context (destroy + create).
	 * @return true if reset (created), false otherwise
	 */
	bool MusicAIManager::reset() {
		destroy();
		return create();
	}

	/**
	 * @brief Checks if the context has been created.
	 * @return true if context exists
	 */
	bool MusicAIManager::isInitialized() const {
		return context_ != nullptr;
	}

	/**
	 * @brief Returns the context pointer. Use with caution.
	 */
	musicai* MusicAIManager::get() {
		return context_;
	}

	// --- Aux methods ---

	static std::vector<std::string> listAudioFilesInDir(const std::string& dirPath) {
		std::vector<std::string> files;
		for (const auto& entry : fs::directory_iterator(dirPath)) {
			if (entry.is_regular_file()) {
				std::string ext = entry.path().extension().string();
				if (ext == ".wav" || ext == ".WAV") {
					files.push_back(entry.path().string());
				}
			}
		}
		return files;
	}

	void MusicAIManager::copy_soundfile_info(SF_INFO* to, const SF_INFO* from) {
		to->samplerate = from->samplerate;
		to->channels = from->channels;
		to->format = from->format;
	}

	bool MusicAIManager::valid_soudfile_info(SF_INFO* info) {
		if (info->channels != EXPECTED_INPUT_CHANNELS) {
			std::cerr << "Can only work with files that follow the following rules:\n";
			std::cerr << "\t- channels: 2\n";
			return false;
		}
		return true;
	}

	// --- Command-related methods ---

	/**
	 * @brief Processes an audio file using a specific model and outputs the result.
	 */
	bool MusicAIManager::process(
		const std::string& modelPath,
		const std::string& inputFilePath,
		const std::string& outputFilePath,
		const std::string& configPath)
	{
		float* input_buffer = NULL;
		size_t input_size = DEFAULT_INPUT_SIZE;
		float* output_buffer = NULL;
		size_t output_size = DEFAULT_OUTPUT_SIZE;

		SF_INFO input_file_info;
		SF_INFO output_file_info;

		memset(&input_file_info, 0, sizeof(SF_INFO));
		memset(&output_file_info, 0, sizeof(SF_INFO));

		SNDFILE* input_file = sf_open(inputFilePath.c_str(), SFM_READ, &input_file_info);
		if (input_file == NULL) {
			fprintf(stderr, "Could not open input file\n");
			return false;
		}

		copy_soundfile_info(&output_file_info, &input_file_info);
		output_file_info.channels = OUTPUT_AUDIO_CHANNELS;
		SNDFILE* output_file = sf_open(outputFilePath.c_str(), SFM_WRITE, &output_file_info);
		if (output_file == NULL) {
			fprintf(stderr, "Could not open output file\n");
			sf_close(input_file);
			return false;
		}
		if (!valid_soudfile_info(&input_file_info)) {
			sf_close(input_file);
			sf_close(output_file);
			return false;
		}
		musicai_model_info model_info;
		musicai_status_result status_result = musicai_get_model_info_from_file(modelPath.c_str(), &model_info);
		if (status_result.code != MUSICAI_STATUS_SUCCESS) {
			fprintf(stderr, "Unable to get model info\n");
			sf_close(input_file);
			sf_close(output_file);
			return false;
		}
		fprintf(stdout, "Model info:\n");
		fprintf(stdout, "Num input channels: %u\n", model_info.num_input_channels);
		fprintf(stdout, "Num output channels: %u\n", model_info.num_output_channels);
		//fprintf(stdout, "Context size: %u\n", model_info.context_size);

		input_buffer = (float*)calloc(1, sizeof(float) * input_size * model_info.num_input_channels);
		if (input_buffer == NULL) {
			fprintf(stderr, "Failed to allocate input buffer\n");
			sf_close(input_file);
			sf_close(output_file);
			return false;
		}
		output_buffer = (float*)calloc(1, sizeof(float) * input_size * model_info.num_output_channels * 4); // 4 output stems
		if (output_buffer == NULL) {
			fprintf(stderr, "Failed to allocate output buffer\n");
			free(input_buffer);
			sf_close(input_file);
			sf_close(output_file);
			return false;
		}
		musicai* context = musicai_create();
		if (context == NULL) {
			fprintf(stderr, "Failed to create musicai context\n");
			free(input_buffer);
			free(output_buffer);
			sf_close(input_file);
			sf_close(output_file);
			return false;
		}

		musicai_processor_config config;
		config.frame_rate = 48000;
		config.preferred_accelerator_unit = MUSICAI_ACCELERATOR_UNIT_CPU; // Mudado de NPU para CPU
		config.preferred_accelerator_unit_index = 0;
		config.preferred_queue_size = 8192; // Reduzido de 80000 para evitar problemas de mem�ria

		musicai_status_result result = musicai_init(context, modelPath.c_str(), &config);
		if (result.code != MUSICAI_STATUS_SUCCESS) {
			fprintf(stdout, "Unable to init musicai context\n");
			free(input_buffer);
			free(output_buffer);
			sf_close(input_file);
			sf_close(output_file);
			musicai_destroy(&context);
			return false;
		}

		size_t frames_read_from_file = 0;
		size_t num_frames_queued = 0;
		size_t partial_frames_queued = 0;
		size_t num_frames_read = 0;

		do {
			memset(input_buffer, 0, sizeof(float) * input_size * model_info.num_input_channels);
			frames_read_from_file = sf_readf_float(input_file, input_buffer, input_size);
			num_frames_queued = 0;
			do {
				// C�digo do processamento
				result = musicai_process(
					context,
					input_buffer + num_frames_queued * model_info.num_input_channels,
					input_size - num_frames_queued,
					&partial_frames_queued
				);
				if (result.code != MUSICAI_STATUS_SUCCESS) {
					fprintf(stderr, "Process failed\n");
					break;
				}
				num_frames_queued += partial_frames_queued;
				result = musicai_retrieve(context, output_buffer, output_size, &num_frames_read);
				if (result.code != MUSICAI_STATUS_SUCCESS) {
					fprintf(stderr, "Retrieve failed\n");
					break;
				}
				sf_writef_float(output_file, output_buffer, num_frames_read);
				fprintf(stderr, "Process - read_frames: %u\n", (unsigned)num_frames_read);
			} while (num_frames_queued < input_size);
			if (result.code != MUSICAI_STATUS_SUCCESS) {
				break;
			}
		} while (frames_read_from_file == input_size);

		if (result.code == MUSICAI_STATUS_SUCCESS) {
			musicai_process(context, NULL, 0, NULL);
			fprintf(stderr, "Process flushing...\n");
			do {
				result = musicai_retrieve(context, output_buffer, output_size, &num_frames_read);
				if (result.code != MUSICAI_STATUS_SUCCESS) {
					fprintf(stderr, "Retrieve failed");
					break;
				}
				sf_writef_float(output_file, output_buffer, num_frames_read);
				fprintf(stderr, "Retrieve - read_frames: %u\n", (unsigned)num_frames_read);
			} while (num_frames_read > 0);
		}
		sf_close(input_file);
		sf_close(output_file);
		free(input_buffer);
		free(output_buffer);
		musicai_destroy(&context);
		fprintf(stdout, "Context destroyed\n");
		return true; // Retorno corrigido para bool
	}

	/**
	 * @brief Processes all audio files in a directory using a list of models.
	 */
	bool MusicAIManager::batchProcess(const std::vector<std::string>& inputFiles,
		const std::vector<std::string>& modelPaths,
		const std::string& outputDirectory,
		const std::string& configPath) {
		namespace fs = std::filesystem;

		if (!fs::exists(outputDirectory)) {
			fs::create_directories(outputDirectory);
		}
		if (inputFiles.empty()) {
			std::cerr << "[MusicAIManager] No input audio files specified.\n";
			return false;
		}
		if (modelPaths.empty()) {
			std::cerr << "[MusicAIManager] No model files specified.\n";
			return false;
		}
		bool anyFailed = false;
		for (const auto& audioFile : inputFiles) {
			std::string baseName = fs::path(audioFile).stem().string();
			for (const auto& modelFile : modelPaths) {
				std::string modelName = fs::path(modelFile).stem().string();
				std::string outputFile = (fs::path(outputDirectory) / (baseName + "__" + modelName + ".wav")).string();
				std::cout << "[MusicAIManager] Processing " << audioFile << " with model " << modelFile
					<< " -> " << outputFile << std::endl;
				if (!process(modelFile, audioFile, outputFile, configPath)) {
					std::cerr << "[MusicAIManager] Failed: " << audioFile << " + " << modelFile << std::endl;
					anyFailed = true;
				}
			}
		}
		if (anyFailed) {
			std::cerr << "[MusicAIManager] Some processing tasks failed." << std::endl;
			return false;
		}
		std::cout << "[MusicAIManager] Batch processing finished." << std::endl;
		return true;
	}

	/**
	 * @brief Retrieves and displays information about a model file.
	 */
	bool MusicAIManager::modelInfo(const std::string& modelPath, musicai_model_info& info) {
		musicai_status_result result = musicai_get_model_info_from_file(modelPath.c_str(), &info);
		return result.code == MUSICAI_STATUS_SUCCESS;
	}

	/**
	 * @brief Checks if a model file can be loaded and initialized.
	 */
	bool MusicAIManager::checkModel(const std::string& modelPath) {
		// Configuração padrão
		musicai_processor_config config;
		memset(&config, 0, sizeof(config));
		config.frame_rate = 48000;
		config.preferred_accelerator_unit = MUSICAI_ACCELERATOR_UNIT_NPU;
		config.preferred_accelerator_unit_index = 0;
		config.preferred_queue_size = 80000;

		musicai* context = musicai_create();
		if (!context) {
			std::cerr << "[CheckModel] musicai_create failed!" << std::endl;
			return false;
		}

		musicai_status_result result = musicai_init(context, modelPath.c_str(), &config);
		if (result.code != MUSICAI_STATUS_SUCCESS) {
			std::cerr << "[CheckModel] FAILED to initialize: " << modelPath << std::endl;
			std::cerr << "[CheckModel] Error code: " << result.code << std::endl;
			musicai_destroy(&context);
			return false;
		}
		musicai_destroy(&context);

		// Sucesso silencioso (pode printar opcionalmente no comando)
		return true;
	}

} // namespace cli