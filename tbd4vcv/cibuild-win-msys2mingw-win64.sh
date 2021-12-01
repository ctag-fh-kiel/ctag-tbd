wget https://vcvrack.com/downloads/Rack-SDK-2.0.0-win.zip
unzip Rack-SDK-2.0.0-win.zip
cd Rack-SDK
export RACK_DIR=`pwd`
cd ..
cd tbd4vcv
mkdir build
cd build
cmake ..
ninja
mkdir tbd4vcv
cp plugin.dll tbd4vcv/
cp ../plugin.json tbd4vcv/
cp ../readme.md tbd4vcv/
cp ../../LICENSE tbd4vcv/
cp -r ../../spiffs_image tbd4vcv/
cp -r ../res tbd4vcv/
mkdir tbd4vcv/sample_rom
cp ../../sample_rom/sample-rom.tbd tbd4vcv/sample_rom/
zip -r tbd4vcv-2.0.0-win64.zip tbd4vcv
