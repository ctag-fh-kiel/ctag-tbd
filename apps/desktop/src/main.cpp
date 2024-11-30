#include <iostream>
#include <boost/program_options.hpp>

#include <tbd/sound_manager.hpp>
#include <tbd/sound_processor/allocator.hpp>
#include <tbd/api/rest_api.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    std::string input_file;
    uint16_t port;

    po::options_description desc(std::string(argv[0]) + " options");
    po::variables_map vm;
    desc.add_options()
            ("help,h", "this help message")
            // ("srom,s", po::value<string>(&sromFile)->default_value("../../sample_rom/sample-rom.tbd"),
            //  "file for sample rom emulation, default ../../sample_rom/sample-rom.tbd")
            // ("list,l", po::bool_switch(&bListSoundCards)->default_value(false), "list sound cards")
            // ("device,d", po::value<int>(&iDeviceNum)->default_value(0), "sound card device id, default 0")
            // ("output,o", po::bool_switch(&bOutputOnly)->default_value(false),
            //  "use output only (if no duplex device available)")
            ("port,p", po::value<uint16_t>(&port)->default_value(2024), "port for REST api"),
            ("wav,w", po::value<std::string>(&input_file),
             "read audio in from wav file (arg), must be 2 channel stereo float32 data, will be cycled through indefinitely");

    store(po::parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // reserve large block of memory before anything else happens
    CTAG::SP::ctagSPAllocator::AllocateInternalBuffer(114688); // TBDings has highest needs of 113944 bytes, take 112k=114688 bytes as default

    tbd::audio::SoundProcessorManager::begin(tbd::audio::AudioParams(input_file));

    tbd::api::RestApiParams params = {.port = port};
    tbd::api::RestApi::begin(params);

    std::cout << "\nRunning ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);

    tbd::api::RestApi::end();

    tbd::audio::SoundProcessorManager::end();
    CTAG::SP::ctagSPAllocator::ReleaseInternalBuffer();
}
