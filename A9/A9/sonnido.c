#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>

#define CHANNELS 2
#define FRAMES   768

int main(int argc, char * argv[]){

    FILE  * rec_file = fopen(argv[1], "w");
    snd_pcm_t * handle;
    snd_pcm_hw_params_t * hw_params;

    /*Abrir tarjeta de audio*/
    int ret = snd_pcm_open(&handle, "hw:0", SND_PCM_STREAM_CAPTURE, 0);

    /**/
    snd_pcm_hw_params_alloca(&hw_params);
    ret = snd_pcm_hw_params_any(handle, hw_params);

    if (ret < 0) {
        fprintf(stderr, "ERROR! Cannot set interleaved mode: %s\n", snd_strerror(ret));
        return ret;
    }

    snd_pcm_format_t format = SND_PCM_FORMAT_S32_LE;

    if( (ret = snd_pcm_hw_params_set_format(handle, hw_params, format)) < 0) {
        printf("ERROR! Cannot set format\n");
        return ret;
    }

    int channels = CHANNELS;

    if( (ret = snd_pcm_hw_params_set_channels(handle, hw_params, channels)) < 0) {
        printf("ERROR! Cannot set Channels\n");
        return ret;
    }

    int rate = 48000;
    if ((ret = snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, 0)) < 0) {
        printf("ERROR! Cannot set Rate %d\n", rate);
        return ret;
    }

    if ((ret = snd_pcm_hw_params(handle, hw_params)) < 0) {
        printf("ERROR! Cannot set hw params\n");
        return ret;
    }

    //
    int size = CHANNELS * FRAMES * sizeof(uint32_t);
    uint32_t * buffer = (uint32_t *)malloc(size);

    for (;;) {
        snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer, FRAMES);
        int n_bytes = fwrite(buffer, 1, size, rec_file);
        fflush(rec_file);
    }


    ret = snd_pcm_close(handle);
    fclose(rec_file);

    return ret;
}