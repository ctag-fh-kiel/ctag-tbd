import os
import csv

bin_size = os.path.getsize("bin/ctag-tbd.bin")
storage = 5000000 + 2 * (3000000 - bin_size)

rom_start = 11534336  # hex: 0xb00000
new_rom_start = rom_start - 2 * (3000000 - bin_size)

new_partitions = []

with open("partitions_example.csv", newline="") as partitions:
    rows = csv.reader(partitions, delimiter=",")
    for row in rows:
        if row[0].startswith("ota_"):
            row[4] = bin_size
        new_partitions.append(row)

with open("partitions_example.csv", "w", newline="") as partitions:
    writer = csv.writer(partitions, delimiter=",")
    for row in new_partitions:
        writer.writerow(row)
    writer.writerow([f"# This partition was generated with the tbd-cloud-compiler. Now with {storage/1000000:.2f}MB (instead of 5MB) for your samples", 4*""])

new_config = []

with open("sdkconfig", "r") as sdkconfig:
    for line in sdkconfig:
        if line.startswith("CONFIG_SAMPLE_ROM_START_ADDRESS="):
            line = f"CONFIG_SAMPLE_ROM_START_ADDRESS={hex(new_rom_start)}\n"
        elif line.startswith("CONFIG_SAMPLE_ROM_SIZE="):
            line = f"CONFIG_SAMPLE_ROM_SIZE={hex(storage)}\n"
        new_config.append(line)

with open("sdkconfig", "w") as sdkconfig:
    sdkconfig.writelines(new_config)
