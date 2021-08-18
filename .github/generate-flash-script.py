import sys
import re

input = sys.stdin.read()
regex = re.compile("/opt/esp/python_env/idf4\.1_py3\.6_env/bin/python \.\./\.\./opt/esp/idf/components/esptool_py/esptool/esptool\.py -p \(PORT\) -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 80m .* build/bootloader/bootloader\.bin .* build/partition_table/partition-table\.bin .* build/ota_data_initial\.bin .* build/ctag-tbd\.bin .* build/storage\.bin")

command = regex.findall(input)[0]
command = command.replace("/opt/esp/python_env/idf4.1_py3.6_env/bin/python", "python")
command = command.replace("../../opt/esp/idf/components/esptool_py/esptool/esptool.py", "$ESP_TOOL_PATH")
command = command.replace("build/", "")
command = command.replace("bootloader/", "")
command = command.replace("partition_table/", "")

with open("flash.sh", "w") as flash_script:
    flash_script.write("#!/bin/sh\n")
    flash_script.write("# Change ESP_TOOL_PATH to the location of your esptool.py\n")
    flash_script.write("# Change PORT to the port your tbd is connected\n")
    flash_script.write('export ESP_TOOL_PATH = "/opt/esp/idf/components/esptool_py/esptool/esptool.py"\n')
    flash_script.write(command)
