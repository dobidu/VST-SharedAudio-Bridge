#ifndef MUSICAI_COMMON_H
#define MUSICAI_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MUSICAI_ENABLE_EXPORT)
#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#define MUSICAI_EXPORT   __attribute__((visibility("default")))
#elif defined(_WIN32)
#define MUSICAI_EXPORT   __declspec(dllexport)
#endif
#else
#define MUSICAI_EXPORT
#endif

typedef enum {
    MUSICAI_STATUS_SUCCESS = 0x0000,
    MUSICAI_STATUS_INVALID_INPUT = 0x0001,
    MUSICAI_STATUS_INVALID_MODEL_TYPE = 0x0002,
    MUSICAI_STATUS_MODEL_LOAD_FAIL = 0x0003,
    MUSICAI_STATUS_RUNNER_INIT_FAIL = 0x0004,
    MUSICAI_STATUS_RUNNER_RUN_FAIL = 0x0005,
    MUSICAI_STATUS_UNKNOWN = 0x0006,
    MUSICAI_STATUS_MEMORY_ALLOCATION = 0x0007,
    MUSICAI_STATUS_VERSION_MISMATCH = 0x0008,
    MUSICAI_STATUS_INVALID_CONFIG = 0x0009,
    MUSICAI_STATUS_FRAMES_MISSING = 0x000a,
    MUSICAI_STATUS_NO_SAMPLE_AVAILABLE = 0x000b,
    MUSICAI_STATUS_NOT_IMPLEMENTED = 0x000c,
    MUSICAI_STATUS_INVALID_STATE = 0x000d
} musicai_status;

/**
 * @brief Status structure containing code and descriptive message
 */
 typedef struct {
    musicai_status code;    /**< Status code indicating operation result */
    const char* message;    /**< Human-readable status message */
} musicai_status_result;

void musicai_fill_status_result(musicai_status_result* status_result, musicai_status code, const char* message);

/**
 * @brief Audio element data type
 */
typedef enum {
    MUSICAI_ELEMENT_DATA_TYPE_FLOAT32 = 1,
    MUSICAI_ELEMENT_DATA_TYPE_UINT8,
    MUSICAI_ELEMENT_DATA_TYPE_INT8,
    MUSICAI_ELEMENT_DATA_TYPE_UINT16,
    MUSICAI_ELEMENT_DATA_TYPE_INT16,
    MUSICAI_ELEMENT_DATA_TYPE_FLOAT16,
    MUSICAI_ELEMENT_DATA_TYPE_UINT32,
    MUSICAI_ELEMENT_DATA_TYPE_INT32
} musicai_element_data_type;

/**
 * @brief Model information structure
 */
 typedef struct {
    uint32_t version;               /**< Model version number */
    uint16_t num_input_channels;    /**< Number of input channels */
    uint16_t num_output_channels;   /**< Number of input channels */
    char description[256];          /**< Model description string */
} musicai_model_info;

typedef enum {
    MUSICAI_ACCELERATOR_UNIT_UNDEFINED = 0,     /**< Pass this value to auto-select the accelerator unit */
    MUSICAI_ACCELERATOR_UNIT_CPU,
    MUSICAI_ACCELERATOR_UNIT_GPU,
    MUSICAI_ACCELERATOR_UNIT_NPU,
} musicai_accelerator_unit;

/**
 * @brief Processor configuration union
 */
typedef struct {
    uint16_t frame_rate;                                    /**< Input frame rate */
    musicai_accelerator_unit preferred_accelerator_unit;    /**< Preferred accelerator unit (NPU, GPU, CPU) */
    uint16_t preferred_accelerator_unit_index;              /**< Index of the accelerator unit. To use the default, set to 0 */
    size_t preferred_queue_size;                            /**< Preferred queue size */
} musicai_processor_config;

/**
 * @brief Gets element data type size
 * @param type Element data type
 * @param size Size of element in bytes
 * @return Status code indicating success or failure
 */
musicai_status musicai_get_element_size(musicai_element_data_type data_type, uint16_t *size);

#ifdef __cplusplus
}
#endif

#endif /* MUSICAI_COMMON_H */

