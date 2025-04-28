import logging


_LOGGER = logging.getLogger(__name__),

def python_dependencies(*packages: list[str]):
    def ensure_package_is_installed(package: str):
        import importlib
        try:
            importlib.import_module(package)
        except ImportError:
            import pip
            pip.main(['install', package])
            _LOGGER.warning(f'installed exterrnal python package {package}')
    for package in packages:
        ensure_package_is_installed(package)

__all__ = [python_dependencies]
