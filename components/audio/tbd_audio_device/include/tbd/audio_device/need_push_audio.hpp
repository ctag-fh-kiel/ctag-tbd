#if !TBD_AUDIO_PULL && !TBD_AUDIO_PUSH
    #error "missing audio input mode configuration, this code requires callback audio input"
#endif

#if !TBD_AUDIO_PUSH
    #error "tbd is not configured to use callback audio input, this code requires callback audio input"
#endif

#if TBD_AUDIO_PULL
    #error "tbd is configured to use polling audio, this code requires callback audio input"
#endif
