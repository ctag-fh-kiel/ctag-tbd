from pathlib import Path
import typer
from functools import cache

from tbdtools.project import get_project_structure, find_project_root

greeting_header_full = r'''
________/\\\\\\\\\__/\\\\\\\\\\\\\\\_____/\\\\\\\\\________/\\\\\\\\\\\\____________/\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\____/\\\\\\\\\\\\____        
 _____/\\\////////__\///////\\\/////____/\\\\\\\\\\\\\____/\\\//////////____________\///////\\\/////__\/\\\/////////\\\_\/\\\////////\\\__       
  ___/\\\/_________________\/\\\________/\\\/////////\\\__/\\\_____________________________\/\\\_______\/\\\_______\/\\\_\/\\\______\//\\\_      
   __/\\\___________________\/\\\_______\/\\\_______\/\\\_\/\\\____/\\\\\\\_________________\/\\\_______\/\\\\\\\\\\\\\\__\/\\\_______\/\\\_     
    _\/\\\___________________\/\\\_______\/\\\\\\\\\\\\\\\_\/\\\___\/////\\\_________________\/\\\_______\/\\\/////////\\\_\/\\\_______\/\\\_    
     _\//\\\__________________\/\\\_______\/\\\/////////\\\_\/\\\_______\/\\\_________________\/\\\_______\/\\\_______\/\\\_\/\\\_______\/\\\_   
      __\///\\\________________\/\\\_______\/\\\_______\/\\\_\/\\\_______\/\\\_________________\/\\\_______\/\\\_______\/\\\_\/\\\_______/\\\__  
       ____\////\\\\\\\\\_______\/\\\_______\/\\\_______\/\\\_\//\\\\\\\\\\\\/__________________\/\\\_______\/\\\\\\\\\\\\\/__\/\\\\\\\\\\\\/___ 
        _______\/////////________\///________\///________\///___\////////////____________________\///________\/////////////____\////////////_____
'''

greeting_header_small = r'''
 ____    ______  ______  ____        ______  ____     ____      
/\  _`\ /\__  _\/\  _  \/\  _`\     /\__  _\/\  _`\  /\  _`\    
\ \ \/\_\/_/\ \/\ \ \L\ \ \ \L\_\   \/_/\ \/\ \ \L\ \\ \ \/\ \  
 \ \ \/_/_ \ \ \ \ \  __ \ \ \L_L      \ \ \ \ \  _ <'\ \ \ \ \ 
  \ \ \L\ \ \ \ \ \ \ \/\ \ \ \/, \     \ \ \ \ \ \L\ \\ \ \_\ \
   \ \____/  \ \_\ \ \_\ \_\ \____/      \ \_\ \ \____/ \ \____/
    \/___/    \/_/  \/_/\/_/\/___/        \/_/  \/___/   \/___/                                                                                                    
'''

greeting_header_tipped = r'''
      ___               ___          ___                                              
     /\__\             /\  \        /\__\                        _____       _____    
    /:/  /       ___  /::\  \      /:/ _/_                 ___  /::\  \     /::\  \   
   /:/  /       /\__\/:/\:\  \    /:/ /\  \               /\__\/:/\:\  \   /:/\:\  \  
  /:/  /  ___  /:/  /:/ /::\  \  /:/ /::\  \             /:/  /:/ /::\__\ /:/  \:\__\ 
 /:/__/  /\__\/:/__/:/_/:/\:\__\/:/__\/\:\__\           /:/__/:/_/:/\:|__/:/__/ \:|__|
 \:\  \ /:/  /::\  \:\/:/  \/__/\:\  \ /:/  /          /::\  \:\/:/ /:/  |:\  \ /:/  /
  \:\  /:/  /:/\:\  \::/__/      \:\  /:/  /          /:/\:\  \::/_/:/  / \:\  /:/  / 
   \:\/:/  /\/__\:\  \:\  \       \:\/:/  /           \/__\:\  \:\/:/  /   \:\/:/  /  
    \::/  /      \:\__\:\__\       \::/  /                 \:\__\::/  /     \::/  /   
     \/__/        \/__/\/__/        \/__/                   \/__/\/__/       \/__/    
'''

greeting_header = '\b\n'.join(greeting_header_tipped.split('\n'))

def common_callback(
    ctx: typer.Context,
    project_dir: Path =  typer.Option(...,'-d', '--project-dir', default_factory=find_project_root)
):
    ctx.obj = get_project_structure(project_dir)
    

@cache  
def get_main() -> typer.Typer:
    """ get the tbd app object
    
        ensures that only one instance is present and allows cnddocs to 
    """
    return typer.Typer(
        name='tbd',
        callback=common_callback, 
        pretty_exceptions_enable=False, help=greeting_header, no_args_is_help=True)


__all__ = ['greeting_header', 'common_callback', 'get_main']