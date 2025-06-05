#ifndef MUSICAI_H
#define MUSICAI_H

#include <musicai/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main context structure for MusicAI
 */
 typedef struct musicai musicai;

 /**
  * @brief Creates a new MusicAI context
  * @return Pointer to newly created context or NULL if creation failed
  */
MUSICAI_EXPORT musicai* musicai_create(void);
 
 /**
  * @brief Destroys a MusicAI context and frees all associated resources
  * @param context Pointer to the context to destroy
  */
MUSICAI_EXPORT void musicai_destroy(musicai** context);

 /**
 * @brief Gets model information from a file without fully loading it
 * @param model_path Path to the model file
 * @param[out] info Pointer to store model information
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_get_model_info_from_file(
    const char* model_path,
    musicai_model_info* info
);

/**
 * @brief Gets model information from a memory buffer
 * @param model_data Pointer to model data in memory
 * @param model_size Size of model data in bytes
 * @param[out] info Pointer to store model information
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_get_model_info_from_memory(
    const void* model_data,
    size_t model_size,
    musicai_model_info* info
);

/**
 * @brief Initializes MusicAI context with a model from file
 * @param context Pointer to the context
 * @param model_path Path to the model file
 * @param config Processor configuration
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_init(
    musicai* context,
    const char* model_path,
    const musicai_processor_config* config
);

/**
 * @brief Initializes MusicAI context with a model from memory
 * @param context Pointer to the context
 * @param model_data Pointer to model data in memory
 * @param model_size Size of model data in bytes
 * @param config Processor configuration
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_init_from_memory(
    musicai* context,
    const void* model_data,
    size_t model_size,
    const musicai_processor_config* config
);

/**
 * @brief Change the current running model on the context
 * @param context Pointer to the context
 * @param model_path Path to the model file
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_load_model(
    musicai* context,
    const char* model_path
);

/**
 * @brief Change the current running model on the context
 * @param context Pointer to the context
 * @param model_data Pointer to model data in memory
 * @param model_size Size of model data in bytes
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_load_model_from_memory(
    musicai* context,
    const void* model_data,
    size_t model_size
);

/**
 * @brief Queues audio frames for subsequent processing.
 *
 * @param context Pointer to the context
 * @param input_buffer Pointer to input audio data containing interleaved float32 audio samples
 * @param size Amount of frames to be processed
 * @param[out] num_queued_frames Amount of frames queued
 * @return Status code indicating success or failure
 *
 * @note This function will block until all or part of the frames can be queued.
 * @note If input_buffer is NULL, the processor is set to drainage mode.
 */
MUSICAI_EXPORT musicai_status_result musicai_process(
    musicai* context,
    const void* input_buffer,
    const size_t size,
    size_t* num_queued_frames
);

/**
 * @brief Retrieve processed audio frames if available.
 *
 * @param context Pointer to the context
 * @param output_buffer Pointer to output audio data
 * @param size Amount of frames to be retrived 
 * @param[out] num_retrieved_frames Amount of frames retrieved
 * @return Status code indicating success or failure
 *
 * @note If the processor is set to drainage mode, this function will block until there are available frames.
 */
MUSICAI_EXPORT musicai_status_result musicai_retrieve(
    musicai* context,
    const void* output_buffer,
    const size_t size,
    size_t* num_retrieved_frames
);

/**
 * @brief Flush the MusicAI buffers.
 * @param context Pointer to the Music AI instance to flush.
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_flush(musicai* context);

/**
 * @brief Gets the amount of frames in the input buffer
 * @param context Pointer to the context
 * @param num_queue Amount of frames in the input buffer
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_get_number_of_queued_frames(musicai* context, uint32_t* num_queued);

/**
 * @brief Gets the amount of frames available in the output buffer
 * @param context Pointer to the context
 * @param num_available Amount of frames available in the output buffer
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_get_number_of_available_frames(musicai* context, uint32_t* num_available);

/**
 * @brief Gets a boolean that indicates if is currently processing
 * @param context Pointer to the context
 * @param processing Indicates if is currently processing
 * @return Status code indicating success or failure
 */
MUSICAI_EXPORT musicai_status_result musicai_is_processing(musicai* context, uint16_t* processing);

/**
 * @brief Shuts down processing and releases resources
 * @param context Pointer to the context
 */
MUSICAI_EXPORT musicai_status_result musicai_shutdown(musicai* context);

#ifdef __cplusplus
}
#endif

#endif /* MUSICAI_H */
