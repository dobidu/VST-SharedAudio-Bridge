#include <musicai/musicai.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

#define EXPECTED_INPUT_CHANNELS     2

#define OUTPUT_AUDIO_CHANNELS       8 // 4 stems 2 channels each

#define DEFAULT_INPUT_SIZE          8192
#define DEFAULT_OUTPUT_SIZE         8192

static void copy_soundfile_info(SF_INFO* to, const SF_INFO* from) {
    to->samplerate = from->samplerate;
    to->channels = from->channels;
    to->format = from->format;
}

static void print_usage() {
    fprintf(stderr, "test <MODEL_PATH> <INPUT_AUDIO_FILE>\n");
}

static int valid_soudfile_info(SF_INFO* info) {
    if (info->channels != EXPECTED_INPUT_CHANNELS) {
        fprintf(stderr, "Can only work with files that follow the following rules:\n");
        fprintf(stderr, "\t- channels: 2\n");
        return 0;
    }

    return 1;
}

int main(int argc, char** argv) {
    float* input_buffer = NULL;
    size_t input_size = DEFAULT_INPUT_SIZE;

    float* output_buffer = NULL;
    size_t output_size = DEFAULT_OUTPUT_SIZE;

    SF_INFO input_file_info;
    SF_INFO output_file_info;

    if (argc < 3) {
        print_usage();
        return 1;
    }

    memset(&input_file_info, 0, sizeof(SF_INFO));
    memset(&output_file_info, 0, sizeof(SF_INFO));

    SNDFILE* input_file = sf_open(argv[2], SFM_READ, &input_file_info);
    if (input_file == NULL) {
        fprintf(stderr, "Could not open input file\n");
        return 1;
    }

    copy_soundfile_info(&output_file_info, &input_file_info);

    output_file_info.channels = OUTPUT_AUDIO_CHANNELS;

    SNDFILE* output_file = sf_open("out.wav", SFM_WRITE, &output_file_info);
    if (output_file == NULL) {
        fprintf(stderr, "Could not open output file\n");
        return 1;
    }

    if (!valid_soudfile_info(&input_file_info)) {
        sf_close(input_file);
        return 1;
    }

    musicai_model_info model_info;

    musicai_status_result status_result = musicai_get_model_info_from_file(argv[1], &model_info);
    if (status_result.code != MUSICAI_STATUS_SUCCESS) {
        fprintf(stderr, "Unable to get model info\n");
        sf_close(input_file);
        sf_close(output_file);
        return 1;
    }

    fprintf(stdout, "Model info:\n");
    fprintf(stdout, "Num input channels: %u\n", model_info.num_input_channels);
    fprintf(stdout, "Num output channels: %u\n", model_info.num_output_channels);
    //fprintf(stdout, "Context size: %u\n", model_info.context_size);

    size_t frames_read_from_file = 0;

    input_buffer = calloc(1, sizeof(float) * input_size * model_info.num_input_channels);
    if (input_buffer == NULL) {
        fprintf(stderr, "Failed to allocate input buffer\n");

        sf_close(input_file);
        sf_close(output_file);

        return 1;
    }

    output_buffer = calloc(1, sizeof(float) * input_size * model_info.num_output_channels * 4); // 4 output stems
    if (output_buffer == NULL) {
        fprintf(stderr, "Failed to allocate output buffer\n");

        free(input_buffer);

        sf_close(input_file);
        sf_close(output_file);

        return 1;
    }

    musicai* context = musicai_create();
    if (context == NULL) {
        fprintf(stderr, "Failed to create musicai context\n");

        free(input_buffer);
        free(output_buffer);

        sf_close(input_file);
        sf_close(output_file);

        return 1;
    }

    musicai_processor_config config;
    config.frame_rate = 48000;
    config.preferred_accelerator_unit = MUSICAI_ACCELERATOR_UNIT_NPU;
    config.preferred_accelerator_unit_index = 0;
    config.preferred_queue_size = 80000;

    musicai_status_result result = musicai_init(context, argv[1], &config);
    if (result.code != MUSICAI_STATUS_SUCCESS) {
        fprintf(stdout, "Unable to init musicai context\n");
        free(input_buffer);
        free(output_buffer);

        sf_close(input_file);
        sf_close(output_file);

        musicai_destroy(&context);

        return 1;
    }

    size_t num_frames_queued = 0;
    size_t partial_frames_queued = 0;
    size_t num_frames_read = 0;

    do {
        memset(input_buffer, 0, sizeof(float) * input_size * model_info.num_input_channels);
        frames_read_from_file = sf_readf_float(input_file, input_buffer, input_size);

        num_frames_queued = 0;

        do {

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

    return 0;
}
