/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "WebServer.hpp"
#include "SimSPManager.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace CTAG::AUDIO;
namespace po = boost::program_options;

int main(int ac, char **av) {
    // parse command line args
    bool bListSoundCards = false;
    bool bOutputOnly = false;
    int iDeviceNum = 0;
    po::options_description desc(string(av[0]) + " options");
    po::variables_map vm;
    try {
        desc.add_options()
                ("help,h", "this help message")
                ("list,l", po::bool_switch(&bListSoundCards)->default_value(false), "list sound cards")
                ("device,d", po::value<int>(&iDeviceNum)->default_value(0), "sound card device id, default 0")
                ("output,o", po::bool_switch(&bOutputOnly)->default_value(false),
                 "use output only (if no duplex device available)");

        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

        if (bListSoundCards) {
            SimSPManager::ListSoundCards();
            return 1;
        }
    } catch (const boost::program_options::required_option &e) {
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        } else {
            throw e;
        }
    }

    SimSPManager::StartSoundProcessor(iDeviceNum, bOutputOnly);

    WebServer webServer;
    webServer.Start();

    std::cout << "\nRunning ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);

    SimSPManager::StopSoundProcessor();

    webServer.Stop();

    return 0;
}
