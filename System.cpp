#include "System.h"



System::System() {
	intcontroller = IntController::getInstance();
	memory = Memory::getInstance();
	processor = Processor::getInstance();
	timer = Timer::getInstance();
	oscore = OSCore::getInstance();

	MMU::init();
	FS::init();
}


System::~System() {
	Timer::release();
	Processor::release();
	OSCore::release();
	IntController::release();
	Memory::release();
}

void System::start() {
	intcontroller->init();
	memory->init(512 * 1024 * 1024);
	processor->init();
	oscore->init();
	timer->init();



	oscore->start();
}
