from pathlib import Path

from esphome.coroutine import coroutine

from tbd_core.buildgen import BuildGenerator

from esphome.core import CORE

NO_EXCEPTION_FLAG = '-fno-exceptions'
CPP_STD_FLAG = '-std=c++20'


class EsphomeBuildGenerator(BuildGenerator):
    @property
    def build_path(self) -> Path:
        return Path(CORE.build_path)

    @property
    def target_platform(self) -> str:
        return CORE.target_platform

    def add_build_flag(self, flag: str) -> None:
        CORE.add_build_flag(flag)

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
        build_flags = [flag for flag in CORE.build_flags if not (flag.startswith('-std=') or flag == NO_EXCEPTION_FLAG)]
        CORE.build_flags = set(build_flags)

        # replace build flags with new ones
        CORE.add_build_flag(CPP_STD_FLAG)
        if exceptions:
            if not CORE.is_host:
                raise ValueError('exceptions can only be disabled on host platform')
        else:
            CORE.add_build_flag(NO_EXCEPTION_FLAG)

    def add_job(self, job) -> None:
        CORE.add_job(job)

    def function_to_job(self, func):
        return coroutine(func)