//
// Created by xuexin.li on 15/5/6.
//

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "as3api.h"
#include "layer3.h"
#include "fmemopen.h"
#include "wave.h"
#include <stdint.h>

#include <AS3/AS3.h>
#include <stdbool.h>
#include "main.h"

shine_config_t config;

static void set_defaults(shine_config_t *config) {
    shine_set_config_mpeg_defaults(&config->mpeg);
}

static void check_config(shine_config_t *config) {
    static char *version_names[4] = {"2.5", "reserved", "II", "I"};
    static char *mode_names[4] = {"stereo", "joint-stereo", "dual-channel", "mono"};
    static char *demp_names[4] = {"none", "50/15us", "", "CITT"};

    printf("MPEG-%s layer III, %s  Psychoacoustic Model: Shine\n",
           version_names[shine_check_config(config->wave.samplerate, config->mpeg.bitr)],
           mode_names[config->mpeg.mode]);
    printf("Bitrate: %d kbps  ", config->mpeg.bitr);
    printf("De-emphasis: %s   %s %s\n",
           demp_names[config->mpeg.emph],
           ((config->mpeg.original) ? "Original" : ""),
           ((config->mpeg.copyright) ? "(C)" : ""));
//    printf("Encoding \"%s\" to \"%s\"\n", infname, outfname);
}

static int write_mp3(long bytes, void *buffer, void *config, FILE *outfile) {
    return fwrite(buffer, sizeof(unsigned char), bytes, outfile) / sizeof(unsigned char);
}

void AS3_Compress() {
    wave_t wave;
    time_t start_time, end_time;
    int16_t buffer[2 * SHINE_MAX_SAMPLES];
    shine_config_t config;
    shine_t s;
    int written;
    unsigned char *data;

    size_t readResult;
    int quite = 0;

    unsigned char *byteArrayWave;
    unsigned long byteArrayWaveSize;

    unsigned char *byteArrayMPEG;
    unsigned long byteArrayMPEGSize;

    FILE *outfile, *infile;

    int stereo = STEREO;
    time(&start_time);

    /* Set the default MPEG encoding paramters - basically init the struct */
    set_defaults(&config);


    int bitr;
    bool force_mono;
    AS3_GetScalarFromVar(bitr, bitrate);
    AS3_GetScalarFromVar(force_mono, mono);
    config.mpeg.bitr = bitr;
    AS3_GetByteArray(input, byteArrayWave, byteArrayWaveSize);

    // read wave as a FILE
    wave.file = fmemopen(byteArrayWave, byteArrayWaveSize, "rb");
    wave.has_seek = 1;
    rewind(wave.file);


    /* Open the input file and fill the config shine_wave_t header */
    if (!wave_open("", &wave, &config, quite)) error("Could not open WAVE file");

    infile = wave.file;

    if (force_mono)
        config.wave.channels = 1;

    /* See if samplerate and bitrate are valid */
    if (shine_check_config(config.wave.samplerate, config.mpeg.bitr) < 0)
        error("Unsupported samplerate/bitrate configuration.");

    byteArrayMPEG = (char *) malloc(sizeof(char) * byteArrayWaveSize);
    if (byteArrayMPEG == NULL) error("Memory error...");
    memset(byteArrayMPEG, 0, byteArrayWaveSize);

    outfile = fmemopen(byteArrayMPEG, byteArrayWaveSize, "wb");
    rewind(outfile);

    /* Set to stereo mode if wave data is stereo, mono otherwise. */
    if (config.wave.channels > 1)
        config.mpeg.mode = stereo;
    else
        config.mpeg.mode = MONO;

    /* Initiate encoder */
    s = shine_initialise(&config);

    // assert(s != NULL);

    /* Print some info about the file about to be created (optional) */
    if (!quite) check_config(&config);

    int samples_per_pass = shine_samples_per_pass(s);

    /* All the magic happens here */
    while (wave_get(buffer, &wave, samples_per_pass)) {
        data = shine_encode_buffer_interleaved(s, buffer, &written);
        if (write_mp3(written, data, &config, outfile) != written) {
            error("shineenc: write error");
        }
    }

    /* Flush and write remaining data. */
    data = shine_flush(s, &written);
    write_mp3(written, data, &config, outfile);

    byteArrayMPEGSize = ftell(outfile);
    rewind(outfile);

    if (!quite) {
        printf("wave file size %d\n", byteArrayWaveSize);
        printf("mpeg file size %d\n", byteArrayMPEGSize);
    }


    AS3_SetByteArray(result, byteArrayMPEG, byteArrayMPEGSize);

    /* Close encoder. */
    shine_close(s);

    /* Close the wave file (using the wav reader) */
    wave_close(&wave);

    /* Close the MP3 file */
    fclose(outfile);

    time(&end_time);
    end_time -= start_time;
    if (!quite)
        printf("Finished in %02ld:%02ld:%02ld (%01.1fx realtime)\n", end_time / 3600, (end_time / 60) % 60,
               end_time % 60, (float) wave.duration / (float) end_time);
    AS3_ReturnAS3Var(result);
}
