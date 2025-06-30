import inspect
from typing import Callable

from tbd_core.reflection import ScopePath, ClassDescription, Reflectables

INDENT_DEPTH = 4

class ArgProxy:
    def __init__(self, type_id: str | ClassDescription, name: str):
        self._type = type_id
        self._name = name

class ReturnProxy:
    def __init__(self, type_id: str | ClassDescription, name: str):
        self._type = type_id
        self._name = name

class VarProxy:
    def __init__(self, type_id: str | ClassDescription, name: str):
        self._type = type_id
        self._name = name

class TypesProxy:
    def __init__(self, ref: Reflectables):
        self._ref = ref

    def __getattribute__(self, item: str):
        if item.startswith("_"):
            return super().__getattribute__(item)

        for t in self._ref.class_list:
            if t.cls_name == item:
                return t
        raise ValueError(f'no such type {item}')

class ProxyCodeGenerator:
    def __init__(self, reflectables: Reflectables):
        self._cmds = []
        self._scope = ScopePath.root()

    def include(self, path: ScopePath | str):
        self._cmds.append(f'#include <{path}>')

    def rel_include(self, path: ScopePath | str):
        self._cmds.append(f'#include "{path}"')

    def ns(self, name: str):
        self._scope = self._scope.add_namespace(name)
        self._cmds.append(f'namespace {self._scope} {{')

    def endns(self):
        self._scope = self._scope.parent
        self._cmds.append(f'}}')

    def var(self, type_id: str | ClassDescription, name: str):
        print(type_id, name)

    def func_body(self, f: Callable):
        spec = inspect.getfullargspec(f)
        for arg in spec.args:
            if arg not in spec.annotations:
                raise ValueError('all c++ arguments must have annotations')



    def log(self):
        print('\n'.join(self._cmds))