from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('jinja2')

from functools import lru_cache
from pathlib import Path
from enum import IntEnum
from esphome.components.tbd_module import new_tbd_component, ComponentInfo, get_generated_include_path
from esphome import pins
import esphome.config_validation as cv
import esphome.codegen as cg
import dataclasses
import jinja2 as ji

AUTO_LOAD = ['tbd_module']

DEFAULT_SAMPLE_RATE = 44100
DEFAULT_CHUNK_SIZE = 32

CONF_CHUNK_SIZE = 'chunk_size'

CONF_PINS = 'pins'
CONF_TYPE = 'type'

CONF_SPI = 'spi'
CONF_SCLK_PIN = 'sclk'
CONF_MOSI_PIN = 'mosi'
CONF_MISO_PIN = 'miso'
CONF_CS_PIN = 'cs'

CONF_I2C = 'i2c'
CONF_SCL_PIN = 'scl'
CONF_SDA_PIN = 'sda'


CONF_I2S = 'i2s'
CONF_MCLK_PIN = 'mclk'
CONF_BCLK_PIN = 'bclk'
CONF_WS_PIN = 'ws'
CONF_DOUT_PIN = 'dout'
CONF_DIN_PIN = 'din'


# SPI device config pinout
SPIPinConfig = {
    # clock pin (SCLK)
    cv.Required(CONF_SCLK_PIN): pins.gpio_output_pin_schema,
    # data out pin (MOSI)
    cv.Required(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
    # data in pin (MISO)
    cv.Required(CONF_MISO_PIN): pins.gpio_input_pin_schema,
    # chip select pin (CS/SS)
    cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
} 

# I2C device config pinout
I2CPinConfig = {
    # clock pin (SCL)
    cv.Required(CONF_SCL_PIN): pins.gpio_output_pin_schema,
    # bidirectional data pin (SDA)
    cv.Required(CONF_SDA_PIN): pins.gpio_output_pin_schema,

} 

# I2C audio data pinout
I2SPinConfig = {
    # master clock (MCLK)
    cv.Optional(CONF_MCLK_PIN): pins.gpio_output_pin_schema,
    # bit clock (BCLK)
    cv.Required(CONF_BCLK_PIN): pins.gpio_output_pin_schema,
    # word select /left-right clock (WS/LRCK)  
    cv.Required(CONF_WS_PIN): pins.gpio_output_pin_schema,
    # data out / DAC data pin (DOUT/DACDAT)
    cv.Required(CONF_DOUT_PIN): pins.gpio_output_pin_schema,
    # data in  / ADC data pin (DIN/ADCDAT)
    cv.Required(CONF_DIN_PIN): pins.gpio_input_pin_schema,
}

class SampleIO(IntEnum):
    WORKER = 0
    CALLBACK = 1

@dataclasses.dataclass
class AudioDevice:
    module: ComponentInfo
    sample_rate: int
    num_channels: int
    chunk_size: int
    sample_io: SampleIO
    subtype: str | None = None


    def add_flag(self):
        name = (self.subtype if self.subtype is not None else self.module.name).upper()
        cg.add_build_flag(f'-DTBD_AUDIO_{name.upper()}=1')

    def add_sample_io_flag(self):
        if self.sample_io == SampleIO.WORKER:
            cg.add_build_flag(f'-DTBD_AUDIO_PULL=1')
        elif self.sample_io == SampleIO.CALLBACK:
            cg.add_build_flag(f'-DTBD_AUDIO_PUSH=1')

    def add_spi(self, config):
        def get_pin_number(key):
            return f'GPIO_NUM_{config[key]['number']}'

        codec_name = self.module.name.upper()  
        cg.add_build_flag(f'-DTBD_{codec_name}_SPI_PIN_CLK={get_pin_number(CONF_SCLK_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_SPI_PIN_MOSI={get_pin_number(CONF_MOSI_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_SPI_PIN_MISO={get_pin_number(CONF_MISO_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_SPI_PIN_CS={get_pin_number(CONF_CS_PIN)}')

    def add_i2c(self, config):
        def get_pin_number(key):
            return f'GPIO_NUM_{config[key]['number']}'

        codec_name = self.module.name.upper()  
        cg.add_build_flag(f'-DTBD_{codec_name}_I2C_PIN_SDA={get_pin_number(CONF_SDA_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_I2C_PIN_SCL={get_pin_number(CONF_SCL_PIN)}')

    def add_i2s(self, config):
        def get_pin_number(key):
            return f'GPIO_NUM_{config[key]['number']}'

        codec_name = self.module.name.upper()  
        if CONF_MCLK_PIN in config:
            cg.add_build_flag(f'-DTBD_{codec_name}_I2S_PIN_MCLK={get_pin_number(CONF_MCLK_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_I2S_PIN_BCLK={get_pin_number(CONF_BCLK_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_I2S_PIN_WS={get_pin_number(CONF_WS_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_I2S_PIN_DOUT={get_pin_number(CONF_DOUT_PIN)}')
        cg.add_build_flag(f'-DTBD_{codec_name}_I2S_PIN_DIN={get_pin_number(CONF_DIN_PIN)}')

    def add_config_header(self):
        base_module = get_tbdd_audio_device_base_module()
        template_path = base_module.path / 'templates'
        header = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape()) \
                    .get_template('audio_settings.j2.hpp') \
                    .render(sample_rate=self.sample_rate, num_channels=self.num_channels, chunk_size=self.chunk_size)
        
        gen_include_path = get_generated_include_path() / 'tbd' / base_module.name
        gen_include_path.mkdir(parents=True, exist_ok=True)
        module_header = gen_include_path / 'audio_settings.hpp'

        with open(module_header, 'w') as f:
            f.write(header)

@dataclasses.dataclass(frozen=True)
class AudioDeviceParams:
    sample_rate: int
    num_channels: int
    chunk_size: int
    sample_io: SampleIO
    subtype: str | None = None

def new_tbd_audio_device(init_file: str, params: AudioDeviceParams):  
    module = new_tbd_component(init_file)

    device = AudioDevice(
        module=module, 
        **dataclasses.asdict(params),
    )
    device.add_flag()
    device.add_sample_io_flag()
    device.add_config_header()
    return device

@lru_cache
def get_tbdd_audio_device_base_module():
    return new_tbd_component(__file__)

async def to_code(config):
    module = get_tbdd_audio_device_base_module()
    module.add_include_dir('tinywav')
    module.add_source_dir('tinywav')

