#include <tbd/sounds/SoundProcessorVoid.hpp>

using namespace tbd::sounds;

void SoundProcessorVoid::Process(const sound_processor::ProcessData&data) {
	// do nothing
}

void SoundProcessorVoid::Init(std::size_t blockSize, void *blockPtr) {

}

SoundProcessorVoid::~SoundProcessorVoid() {
}
