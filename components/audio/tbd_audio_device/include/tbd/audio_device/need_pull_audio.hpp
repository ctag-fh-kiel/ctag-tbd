#if !TBD_AUDIO_PULL && !TBD_AUDIO_PUSH
    #error "missing audio input mode configuration, this code requires polling audio input"
#endif

#if !TBD_AUDIO_PULL
    #error "tbd is not configured to use polling audio input, this code requires polling audio input"
#endif

#if TBD_AUDIO_PUSH
    #error "tbd is configured to use callback audio, this code requires polling audio input"
#endif
