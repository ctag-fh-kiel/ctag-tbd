#include <tbd/sounds/ctagSoundProcessorVoid.hpp>

using namespace CTAG::SP;

void ctagSoundProcessorVoid::Process(const ProcessData &data) {
}

void ctagSoundProcessorVoid::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);
}

ctagSoundProcessorVoid::~ctagSoundProcessorVoid() {
}

void ctagSoundProcessorVoid::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("dummy", [&](const int val){ dummy = val;});
	pMapTrig.emplace("dummy", [&](const int val){ trig_dummy = val;});
	isStereo = false;
	id = "Void";
	// sectionCpp0
}