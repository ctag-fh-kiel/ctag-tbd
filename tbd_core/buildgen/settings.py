from dataclasses import dataclass

from .registry import generated_tbd_global


SETTINGS_DOMAIN = 'settings'


@dataclass
class BuildSettings:
    use_symlinks: bool = True


@generated_tbd_global(SETTINGS_DOMAIN)
def get_build_settings() -> BuildSettings:
    return BuildSettings()

__all__ = ['get_build_settings']
