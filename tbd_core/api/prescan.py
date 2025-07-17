from pathlib import Path

from .idc_interfaces import idc_from_function, Event
from tbd_core.reflection.registry import ReflectableFinder


def find_events(*source_files: Path | str, base_path: Path | str | None = None, collector: ReflectableFinder) -> list[Event]:
    base_path = Path(base_path) if base_path else None

    events = []
    for source_file in source_files:
        if base_path:
            source_file = base_path / source_file
        events = [*events, *_find_event(source_file, collector)]
    return events

def _find_event(source_file: Path, collector: ReflectableFinder) -> list[Event]:
    source_file = Path(source_file)

    events = []
    if not source_file.exists():
        raise ValueError(f'endpoint source {source_file} does not exist')
    collector.add_from_file(source_file)
    for func in collector.get_reflectables().functions():
        if not (idc := idc_from_function(func)):
            continue
        if not isinstance(idc, Event):
            continue
        events.append(idc)

    return events


__all__ = ['find_events']
