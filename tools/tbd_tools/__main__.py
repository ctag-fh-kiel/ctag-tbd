#!/usr/bin/env python

import importlib
from pathlib import Path
import sys
from loguru import logger


def import_cli_relative():
    """ load the TBD CLI callable via relative import
    
        This is intended for use from within the "tbd_tools" python package as in
        
        .. code::block:: shell 

            python -m some_path/tbd_tools

        :return"
    """
    
    try:
        from .tbd_tool import tbd_tool
        tool_cli = tbd_tool
        logger.debug('loaded CLI from relative import')
        return tool_cli
    except (ImportError, FileNotFoundError) as error:
        logger.debug(f'relative import failed {error}')
        return None

def import_cli_absolute():
    """ load the TBD CLI callable by files system path based import
    
        This is intended to be used when ``__main__.py`` gets called as a script, 
        as in 
        
        .. code::block:: shell 

            python some_path/tbd_tools/__main__.py

        :return"
    """
        
    try:
        tools_module_path = Path(__file__).parent
        print(tools_module_path)
        tools_module_name = tools_module_path.name
        print(tools_module_name)
        init_path = tools_module_path / '__init__.py'
        spec = importlib.util.spec_from_file_location(tools_module_name, init_path)
        tools_module = importlib.util.module_from_spec(spec)
        sys.modules[tools_module_name] = tools_module
        spec.loader.exec_module(tools_module)
        tool_cli = tools_module.tbd_tool
        logger.debug('loaded CLI from path')
        return tool_cli
    except (ImportError, FileNotFoundError) as error:
        logger.debug(f'relative import failed {error}')
        return None


def main():
    """ robust entry point for TBD cli

        Unfortunately we can't fully avoid context in which we need to directly invoke ``__main__.py``
        as a script. We explicitly allow this and make sure relative imports work with the following logic:

        first: if relative imports work: 
            - all is fine we seem to be in a module context
            - run CLI
        
        otherwise: 
            - ``__main__.py`` seems to have been called a script
            - given the location of this file, attempt to go full circle and load the containing folder as a module
            - get the CLI object from the loaded module
            - run CLI


    """
    if (tool_cli := import_cli_relative()):
        tool_cli()
    elif (tool_cli := import_cli_absolute()):
        tool_cli()
    else:
        logger.error('failed to load TBD CLI module')


if __name__ == '__main__':
    main()
        

__all__ = []