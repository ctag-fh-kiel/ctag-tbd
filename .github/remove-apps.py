import sys

apps = sys.stdin.read().split("#")
print("The following apps will not be included in your custom firmware:")
for app in apps:
    print(app)