#ifndef __DA_SEPARATE_COMMON_H_
#define __DA_SEPARATE_COMMON_H_


enum da_separate_mute {
    UNMUTE_PGA = 0,
    MUTE_PGA = 1,
};

extern void audio_codec_mute_pga(bool mute);

#endif

