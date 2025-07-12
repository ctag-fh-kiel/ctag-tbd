from enum import unique, IntEnum


@unique
class GenerationStages(IntEnum):
    """ Priority of tbd build setup stages.

        Stages with higher priority run first with default value `0.0`.

        TBD does a lot of processing and code generation. This processing needs to be done in a defined order, since
        the output of one processing stage may be required by a subsequent stage. For this purpose esphome allows
        build jobs to be assigned priorities. The default priority assigned to by esphome for calling `to_code` of
        each component is `0.0`.

        The priority represent the following stages:

        `DEFAULT`: Components registering and config stage.
            Priority of `to_code` call if not specified. Normal components `to_code` module level function
            evaluates the component config if present and registers the component, including sources, include paths
            and defines.

        `COMPONENT`: Component registry build setup stage.
            Component registry gets evaluated and all code is copied or symlinked to build dir. Additionally, the
            PlatformIO config file is populated with defines, include paths and libraries.

        'REFLECTION': Reflection parsing and metadata generation stage.
            Functionality like the plugin registry, parse C++ code and generate additional wrapper or metadata files.

        'API': Api registry stage.
            With all other code generation and the build set up, the API registry will parse selected files for API
            endpoint declarations and read DTO definitions for endpoint and DTO code generation.
    """

    DEFAULT       =  0
    REFLECTION    = -10
    TESTS         = -15
    PLUGINS       = -20
    ERRORS        = -25
    FINALIZE      = -30
    API           = -35
    SERIALIZATION = -40
    COMPONENTS    = -45
    CLIENTS       = -50


__all__ = ['GenerationStages']