wget https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.bz2
tar --bzip2 -xf boost_1_77_0.tar.bz2
cd boost_1_77_0
export BOOST_ROOT=`pwd`
export BOOST_LIBRARYDIR=`pwd`/stage/lib
./bootstrap.sh
./b2 cxxflags=-fPIC cflags=-fPIC --with-system --with-filesystem --with-thread -a threading=multi link=static
cd ..
wget https://vcvrack.com/downloads/Rack-SDK-2.0.0-lin.zip
unzip Rack-SDK-2.0.0-lin.zip
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