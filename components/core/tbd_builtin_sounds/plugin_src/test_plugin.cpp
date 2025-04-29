#include <tbd/sounds/test_plugin.hpp>

using namespace tbd::sounds;

void TestPlugin::Process(const audio::ProcessData&data) {
	// do nothing
}

void TestPlugin::Init(std::size_t blockSize, void *blockPtr) {

}

TestPlugin::~TestPlugin() {
}
