import logging


_LOGGER = logging.getLogger(__name__)


def python_dependencies(*packages: list[str | tuple[str, str]]):
    def ensure_package_is_installed(package: str):
        if isinstance(package, tuple):
            import_name, package_name = package
        else:
            import_name = package
            package_name = package

        import importlib
        try:    
            importlib.import_module(import_name)
        except ImportError:
            import pip
            pip.main(['install', package_name])
            _LOGGER.warning(f'installed exterrnal python package {package_name}')
            
    for package in packages:
        ensure_package_is_installed(package)

__all__ = ['python_dependencies']
