HEADER_EXTENSION = ['hpp', 'hh', 'h', 'inl', 'hxx', 'h++', 'hp', 'tcc']
SOURCE_EXTENSIONS = ['cpp', 'cc', 'c', 'cxx', 'c++']
ALL_EXTENSIONS = HEADER_EXTENSION + SOURCE_EXTENSIONS


def header_extensions() -> list[str]:
    return list(HEADER_EXTENSION)


def header_file_patterns() -> list[str]:
    return [f'*.{extension}' for extension in HEADER_EXTENSION]


def source_extensions() -> list[str]:
    return list(SOURCE_EXTENSIONS)


def source_file_patterns() -> list[str]:
    return [f'*.{extension}' for extension in SOURCE_EXTENSIONS]


def cpp_extensions() -> list[str]:
    return list(ALL_EXTENSIONS)


def cpp_patterns() -> list[str]:
    return [f'*.{extension}' for extension in ALL_EXTENSIONS]


__all__ = [
    'header_extensions',
    'header_file_patterns',
    'source_extensions',
    'source_file_patterns',
    'cpp_extensions',
    'cpp_patterns',
]