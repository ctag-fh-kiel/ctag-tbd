from esphome.components.tbd_module import new_tbd_component

from esphome.core import CORE
import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.git as git
from esphome.const import CONF_REFRESH
from pathlib import Path

AUTO_LOAD = ['tbd_module']
if CORE.is_esp32:
    DEPENDENCIES = ['esp32']
elif CORE.is_host:
    DEPENDENCIES = ['host']
else:
    raise ValueError(f'the tbd_dsp module does not support {CORE.target_platform} platform')


ESP_DSP_SRC_FILES = [
    '/modules/common/misc/dsps_pwroftwo.cpp',
    '/modules/dotprod/float/dsps_dotprod_f32_ansi.c',
    '/modules/dotprod/float/dsps_dotprode_f32_ansi.c',
    '/modules/dotprod/fixed/dsps_dotprod_s16_ansi.c',
    '/modules/matrix/float/dspm_mult_f32_ansi.c',
    '/modules/matrix/fixed/dspm_mult_s16_ansi.c',
    '/modules/math/mulc/float/dsps_mulc_f32_ansi.c',
    '/modules/math/addc/float/dsps_addc_f32_ansi.c',
    '/modules/math/add/float/dsps_add_f32_ansi.c',
    '/modules/math/sub/float/dsps_sub_f32_ansi.c',
    '/modules/math/mul/float/dsps_mul_f32_ansi.c',
    '/modules/support/misc/dsps_d_gen.c',
    '/modules/support/misc/dsps_h_gen.c',
    '/modules/support/misc/dsps_tone_gen.c',
    '/modules/support/view/dsps_view.cpp',
    '/modules/windows/hann/float/dsps_wind_hann_f32.c',
    '/modules/windows/blackman/float/dsps_wind_blackman_f32.c',
    '/modules/windows/blackman_harris/float/dsps_wind_blackman_harris_f32.c',
    '/modules/windows/blackman_nuttall/float/dsps_wind_blackman_nuttall_f32.c',
    '/modules/windows/nuttall/float/dsps_wind_nuttall_f32.c',
    '/modules/windows/flat_top/float/dsps_wind_flat_top_f32.c',
    '/modules/conv/float/dsps_conv_f32_ansi.c',
    '/modules/conv/float/dsps_corr_f32_ansi.c',
    '/modules/conv/float/dsps_ccorr_f32_ansi.c',
    '/modules/iir/biquad/dsps_biquad_f32_ansi.c',
    '/modules/iir/biquad/dsps_biquad_gen_f32.c',
    '/modules/fir/float/dsps_fir_f32_ansi.c',
    '/modules/fir/float/dsps_fir_init_f32.c',
    '/modules/fir/float/dsps_fird_f32_ansi.c',
    '/modules/fir/float/dsps_fird_init_f32.c',
]

ESP_DSP_INC_DIRS = [
    '/modules/dotprod/include',
    '/modules/support/include',
    '/modules/windows/include',
    '/modules/windows/hann/include',
    '/modules/windows/blackman/include',
    '/modules/windows/blackman_harris/include',
    '/modules/windows/blackman_nuttall/include',
    '/modules/windows/nuttall/include',
    '/modules/windows/flat_top/include',
    '/modules/iir/include',
    '/modules/fir/include',
    '/modules/math/include',
    '/modules/math/add/include',
    '/modules/math/sub/include',
    '/modules/math/mul/include',
    '/modules/math/addc/include',
    '/modules/math/mulc/include',
    '/modules/matrix/include',
    '/modules/fft/include',
    '/modules/dct/include',
    '/modules/conv/include',
    '/modules/common/include',
]

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_REFRESH, default="1d"): cv.All(
        cv.string, cv.source_refresh
    ),
})

def to_code(config):
    component = new_tbd_component(__file__)
    
    ESP_DSP_NAME='esp-dsp'
    ESP_DSP_URL='https://github.com/espressif/esp-dsp.git'
    ESP_DSP_PATH = Path('modules') / 'iir' / 'include'
    ESP_DSP_VERSION = 'v1.6.1'

    if CORE.is_esp32: 
        from esphome.components.esp32 import add_idf_component
        cg.add_define('USE_ESP_DSP')
        add_idf_component(
            name=ESP_DSP_NAME,
            repo=ESP_DSP_URL,
            ref=ESP_DSP_VERSION,
            # refresh='1day'
        )
        
        dsp_include = Path(CORE.build_path) / 'components' / 'esp-dsp' / ESP_DSP_PATH
        cg.add_build_flag(f'-I{dsp_include}')

    elif CORE.is_host:
        repo_dir, *_ = git.clone_or_update(
            url=ESP_DSP_URL,
            ref=ESP_DSP_VERSION,
            refresh=config[CONF_REFRESH],
            domain=component.full_name,
        )
        for subdir in ESP_DSP_INC_DIRS:
            cg.add_build_flag(f'-I{repo_dir}/{subdir}')
        

        src_files = [f'+<{repo_dir}/{src_file}>' for src_file in ESP_DSP_SRC_FILES]
        CORE.add_platformio_option('build_src_filter', src_files)
        module_dir = repo_dir / ESP_DSP_PATH

        print(module_dir)

    