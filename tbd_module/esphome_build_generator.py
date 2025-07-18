from pathlib import Path
from typing import override

from esphome.coroutine import coroutine

from tbd_core.buildgen import BuildGenerator
from tbd_core.buildgen.build_deps import ExternalDependency, ExternalLibrary, SystemLibrary

import esphome.core as ehc
from esphome.writer import INI_AUTO_GENERATE_BEGIN, INI_AUTO_GENERATE_END, INI_BASE_FORMAT

NO_EXCEPTION_FLAG = '-fno-exceptions'
CPP_STD_FLAG = '-std=c++20'


class EsphomeBuildGenerator(BuildGenerator):
    @override
    def write_config(self):
        if not self._platformio_raw:
            return

        with open(self.build_path / 'platformio.ini', 'w') as f:
            lines = [*INI_BASE_FORMAT, *self._platformio_raw, '', INI_AUTO_GENERATE_BEGIN, INI_AUTO_GENERATE_END]
            data = '\n'.join(lines)
            f.write(data)

    def add_library(self, lib: ExternalDependency) -> None:
        match lib:
            case ExternalLibrary():
                ehc.CORE.add_library(ehc.Library(lib.name, lib.version, lib.repository))
            case SystemLibrary():
                ehc.CORE.add_build_flag(f'-l{lib.name}')

    @property
    def build_path(self) -> Path:
        return Path(ehc.CORE.build_path)

    @property
    def target_platform(self) -> str:
        return ehc.CORE.target_platform

    def add_build_flag(self, flag: str) -> None:
        ehc.CORE.add_build_flag(flag)

    def set_compiler_options(self, *, std: int = 20, exceptions: bool = False) -> None:
        """ Set compiler options.

            This solution is very hacky and should be changed as soon as possible.

            :warning: Enabling exception only available on host platform.

            On the host platform some libraries like rtaudio will simply not work without
            having exceptions enabled, so we trick esphome into enabling them.

            ..note:
                exception flag is disabled in esphome coroutine with priority `100.0` (`to_code`
                methods are executed with default priority `0.0`). Make sure to not call this method
                from any prority `>= 100.0`.
        """

        # throw out build flags we want to overwrite
        build_flags = [flag for flag in ehc.CORE.build_flags if not (flag.startswith('-std=') or flag == NO_EXCEPTION_FLAG)]
        ehc.CORE.build_flags = set(build_flags)

        build_unflags = [flag for flag in ehc.CORE.build_unflags if not (flag.startswith('-std=') or flag == NO_EXCEPTION_FLAG)]
        ehc.CORE.build_unflags = set(build_unflags)

        # replace build flags with new ones
        ehc.CORE.add_build_flag(CPP_STD_FLAG)
        if exceptions:
            if not ehc.CORE.is_host:
                raise ValueError('exceptions can only be disabled on host platform')
        else:
            ehc.CORE.add_build_flag(NO_EXCEPTION_FLAG)

    def add_platformio_option(self, key: str, value: str | list[str]) -> None:
        ehc.CORE.add_platformio_option(key, value)

    def add_job(self, job) -> None:
        ehc.CORE.add_job(job)

    def function_to_job(self, func):
        return coroutine(func)