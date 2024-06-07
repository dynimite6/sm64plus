#ifdef __MINGW32__
#include "SDL.h"
#elif TARGET_MACOS
#include <SDL.h>
#else
#include "SDL2/SDL.h"
#endif

#include "audio_api.h"

#include "../../game/settings.h"

static SDL_AudioDeviceID dev;

static bool audio_sdl_init(void) {
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return false;
    }
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 32000;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 512;
    want.callback = NULL;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (dev == 0) {
        fprintf(stderr, "SDL_OpenAudio error: %s\n", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(dev, 0);
    return true;
}

static int audio_sdl_buffered(void) {
    return SDL_GetQueuedAudioSize(dev) / 4;
}

static int audio_sdl_get_desired_buffered(void) {
    return 1100;
}

static void audio_sdl_play(const uint8_t *buf, size_t len) {
    if (configOverallVolume > 0 && audio_sdl_buffered() < 6000) {
#if TARGET_MACOS
        SDL_QueueAudio(dev, buf, len);
#else
        uint8_t *mix_buf[len];
        SDL_memset(mix_buf, 0, len);
        SDL_MixAudioFormat((Uint8*)mix_buf, buf, AUDIO_S16, len, SDL_MIX_MAXVOLUME * configOverallVolume);
        SDL_QueueAudio(dev, mix_buf, len);
#endif
    }
}

struct AudioAPI audio_sdl = {
    audio_sdl_init,
    audio_sdl_buffered,
    audio_sdl_get_desired_buffered,
    audio_sdl_play
};
