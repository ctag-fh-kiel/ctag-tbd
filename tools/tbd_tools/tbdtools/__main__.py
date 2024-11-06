#!/usr/bin/env python

from pathlib import Path
import sys
from loguru import logger


def import_cli_relative():
    """ load the TBD CLI callable via relative import
    
        This is intended for use from within the "tbd_tools" python package as in
        
        .. code::block:: shell 

            python -m some_path/tbd_tools

        :return: main cli class
    """
    
    try:
        from .cli import tbd_cli
        logger.debug('loaded CLI from relative import')
        return tbd_cli
    except (ImportError, FileNotFoundError) as error:
        logger.debug(f'relative import failed {error}')
        return None


def import_cli_absolute():
    """ load the TBD CLI callable by files system path based import
    
        This is intended to be used when ``__main__.py`` gets called as a script, 
        as in 
        
        .. code::block:: shell 

            python some_path/tbd_tools/__main__.py

        :return: main cli class
    """
        
    try:
        from tbdtools import tbd_cli
        return tbd_cli
    except (ImportError, FileNotFoundError) as error:
        logger.debug(f'absolute import failed {error}')
        return None
    

def import_cli_with_added_module_search_path():
    """ fallback if dtbtools is not in module search path
    
        This adds tbd_tools to the python search path, to ensure the CLI can be run
        even if the tbd_tools root dir is not in the python module search path.

        :return: main cli class
    """
        
    try:
        new_search_path = str(Path(__file__).parent.parent)
        sys.path.append(new_search_path)

        from tbdtools import tbd_cli
        return tbd_cli
    except (ImportError, FileNotFoundError) as error:
        logger.debug(f'absolute import failed {error}')
        return None


def main():
    """ robust entry point for TBD cli

        There are certain use cases in which we need to directly invoke ``__main__.py`` as a 
        script. To make the CLI entry point as robust as possible we want all combinations 
        of the following to work:

        - invoked as module or ``__main__.py`` invoked as script
        - tbdtools in or not in module search path

        The follwing three attempts to load the main CLI are made:

        1. relative imports work: 
        
            - cli got invoked as a module
            - ``tbdtools`` is in module search path
            - run CLI
        
        2. module import of ``tbdtools`` works:
         
            - ``__main__.py`` seems to have been called a script
            - ``tbdtools`` is in module search path
            - run CLI

        3. adding parent folder of ``__main__.py`` and import ``tbdtools`` works
         
            - ``tbdtools`` was not in module search path
            - run CLI    
    """
    
    if (tbd_cli := import_cli_relative()):
        tbd_cli()
    elif (tbd_cli := import_cli_absolute()):
        tbd_cli()
    elif (tbd_cli := import_cli_with_added_module_search_path()):
        tbd_cli()
    else:
        logger.error('failed to load TBD CLI module')


if __name__ == '__main__':
    main()
        

__all__ = []
