import sys
import os

apps = sys.stdin.read().split("#")
apps = map(lambda x: x.strip(), apps)
print("The following apps will not be included in your custom firmware:")
for app in apps:
    os.remove(f"spiffs_image/data/sp/mp-{app}.jsn")
    os.remove(f"spiffs_image/data/sp/mui-{app}.jsn")
    os.remove(f"components/ctagSoundProcessor/ctagSoundProcessor{app}.hpp")
    os.remove(f"components/ctagSoundProcessor/ctagSoundProcessor{app}.cpp")

