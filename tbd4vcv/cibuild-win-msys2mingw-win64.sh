wget https://vcvrack.com/downloads/Rack-SDK-1.1.6.zip
unzip Rack-SDK-1.1.6.zip
cd Rack-SDK
export RACK_DIR=`pwd`
cd ..
cd tbd4vcv
mkdir build
cd build
cmake ..
ninja
cpack