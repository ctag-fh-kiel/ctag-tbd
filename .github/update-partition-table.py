import os
import csv
import sys

bin_size = os.path.getsize("bin/ctag-tbd.bin")
storage = 8000000 + (3000000 - bin_size)

rom_start = 11534336 - 3000000 # hex: 0xb00000
new_rom_start = rom_start - (3000000 - bin_size)

new_partitions = []

with open("partitions_example.csv", newline="") as partitions:
    rows = csv.reader(partitions, delimiter=",")
    for row in rows:
        if row[0].startswith("ota_0"):
            row[4] = bin_size
        if not row[0].startswith("ota_1"):
            new_partitions.append(row)

with open("partitions_example.csv", "w", newline="") as partitions:
    writer = csv.writer(partitions, delimiter=",")
    for row in new_partitions:
        writer.writerow(row)
    writer.writerow([f"# This partition was generated with the tbd-cloud-compiler. Now with {storage/1000000:.2f}MB (instead of 5MB) for your samples", 4*""])

new_config = []
platform = sys.stdin.read()

with open(f"sdkconfig.defaults.{platform}", "r") as sdkconfig:
    for line in sdkconfig:
        if line.startswith("CONFIG_SAMPLE_ROM_START_ADDRESS="):
            line = f"CONFIG_SAMPLE_ROM_START_ADDRESS={hex(new_rom_start)}\n"
        elif line.startswith("CONFIG_SAMPLE_ROM_SIZE="):
            line = f"CONFIG_SAMPLE_ROM_SIZE={hex(storage)}\n"
        new_config.append(line)

with open("sdkconfig.defaults", "w") as sdkconfig:
    sdkconfig.writelines(new_config)

new_html = ""

with open("spiffs_image/www/config.html", "r") as html:
    new_html = html.read().replace("""    <ons-list-item>
        <div style="text-align: left;">
            <ons-progress-bar id="fw-progress" value="0" secondary-value="0"></ons-progress-bar>
            <label for="app-binary">ctag-tbd.bin</label><input id="app-binary" type="file"/>
            <label for="spiffs-binary">storage.bin</label><input id="spiffs-binary" type="file"/>
            <ons-button id="upgrade-firmware" style="margin-top: 10px;">Upgrade Firmware</ons-button>
        </div>
    </ons-list-item>""", """    <ons-list-item>
        <div style="text-align: left;">
            <p>No firmware upgrade possible via WIFI with firmware from <a href="fxwiegand.github.io/tbd-cloud-compiler" target="_blank">the tbd-cloud-compiler</a>.</p>
        </div>
    </ons-list-item>""")

with open("spiffs_image/www/config.html", "w") as html:
    html.writelines(new_html)
