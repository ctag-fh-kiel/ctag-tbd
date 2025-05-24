import logging


_LOGGER = logging.getLogger(__name__)


def ensure_package_is_installed(import_name: str, package_name: str):
    import importlib
    try:
        importlib.import_module(import_name)
    except ImportError:
        import pip
        pip.main(['install', package_name])
        _LOGGER.warning(f'installed exterrnal python package {package_name}')


def python_dependencies(*packages: str | tuple[str, str]):
    for package in packages:
        match package:
            case (import_name, package_name):
                ensure_package_is_installed(import_name, package_name)
            case str():
                ensure_package_is_installed(package, package)
            case _:
                raise ValueError(f'invalid python dependency description {package}')

__all__ = ['python_dependencies']
