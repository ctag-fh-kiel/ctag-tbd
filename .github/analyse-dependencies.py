import os
import sys

dependecies = {}

for file in os.listdir("components/ctagSoundProcessor/"):
    if file.startswith("ctagSoundProcessor") and file.endswith(".cpp"):
        processor = file.removeprefix("ctagSoundProcessor").removesuffix(".cpp")
        dependecies[processor] = set()
        with open(f"components/ctagSoundProcessor/{file}", "r") as f:
            for line in f:
                if line.startswith("#include"):
                    dep = line.removeprefix("#include").strip()
                    if f"ctagSoundProcessor{processor}.hpp" not in dep:
                        dependecies[processor].add(dep)


apps = sys.stdin.read().split("#")
apps = list(map(lambda x: x.strip(), apps))
apps = list(set(dependecies.keys()) - set(apps))

used_dependecies = set(sum([list(dependecies[app]) for app in apps], []))

additional_apps = []

for processor, deps in dependecies.items():
    if deps.issubset(used_dependecies):
        additional_apps.append(processor)


result = set(dependecies.keys()) - set(apps + additional_apps)
sys.stdout.write("#".join(result))
