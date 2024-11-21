set(ESP_DSP_DIR "${CMAKE_SOURCE_DIR}/managed_components/espressif__esp-dsp")
tbd_logw("ESP_DSP ${ESP_DSP_DIR}")


set(ESP_DSP_SRC_FILES  
        "${ESP_DSP_DIR}/modules/common/misc/dsps_pwroftwo.cpp"
        "${ESP_DSP_DIR}/modules/dotprod/float/dsps_dotprod_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/dotprod/float/dsps_dotprode_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/dotprod/fixed/dsps_dotprod_s16_ansi.c"
        "${ESP_DSP_DIR}/modules/matrix/float/dspm_mult_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/matrix/fixed/dspm_mult_s16_ansi.c"
        "${ESP_DSP_DIR}/modules/math/mulc/float/dsps_mulc_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/addc/float/dsps_addc_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/add/float/dsps_add_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/sub/float/dsps_sub_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/mul/float/dsps_mul_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_d_gen.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_h_gen.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_tone_gen.c"
        "${ESP_DSP_DIR}/modules/support/view/dsps_view.cpp"
        "${ESP_DSP_DIR}/modules/windows/hann/float/dsps_wind_hann_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman/float/dsps_wind_blackman_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman_harris/float/dsps_wind_blackman_harris_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman_nuttall/float/dsps_wind_blackman_nuttall_f32.c"
        "${ESP_DSP_DIR}/modules/windows/nuttall/float/dsps_wind_nuttall_f32.c"
        "${ESP_DSP_DIR}/modules/windows/flat_top/float/dsps_wind_flat_top_f32.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_conv_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_corr_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_ccorr_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/iir/biquad/dsps_biquad_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/iir/biquad/dsps_biquad_gen_f32.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fir_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fir_init_f32.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fird_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fird_init_f32.c")

set(ESP_DSP_INC_DIRS 
        "${ESP_DSP_DIR}/modules/dotprod/include"
        "${ESP_DSP_DIR}/modules/support/include"
        "${ESP_DSP_DIR}/modules/windows/include"
        "${ESP_DSP_DIR}/modules/windows/hann/include"
        "${ESP_DSP_DIR}/modules/windows/blackman/include"
        "${ESP_DSP_DIR}/modules/windows/blackman_harris/include"
        "${ESP_DSP_DIR}/modules/windows/blackman_nuttall/include"
        "${ESP_DSP_DIR}/modules/windows/nuttall/include"
        "${ESP_DSP_DIR}/modules/windows/flat_top/include"
        "${ESP_DSP_DIR}/modules/iir/include"
        "${ESP_DSP_DIR}/modules/fir/include"
        "${ESP_DSP_DIR}/modules/math/include"
        "${ESP_DSP_DIR}/modules/math/add/include"
        "${ESP_DSP_DIR}/modules/math/sub/include"
        "${ESP_DSP_DIR}/modules/math/mul/include"
        "${ESP_DSP_DIR}/modules/math/addc/include"
        "${ESP_DSP_DIR}/modules/math/mulc/include"
        "${ESP_DSP_DIR}/modules/matrix/include"
        "${ESP_DSP_DIR}/modules/fft/include"
        "${ESP_DSP_DIR}/modules/dct/include"
        "${ESP_DSP_DIR}/modules/conv/include"
        "${ESP_DSP_DIR}/modules/common/include")

# file(GLOB ESP_DSP_SRC_FILES 
#         modules/**/*.c
# )

add_library(tbd_dsp STATIC ${ESP_DSP_SRC_FILES})
target_include_directories(tbd_dsp PUBLIC include ${ESP_DSP_INC_DIRS})
