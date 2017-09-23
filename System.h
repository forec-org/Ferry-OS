#pragma once

#include "OSCore.h"
#include "Processor.h"
#include "IntController.h"
#include "Timer.h"
#include "Memory.h"
#include "./include/device/memory/mmu.h"
#include "./include/device/fs/fs.h"

class System {
private:
	Timer			*timer;
	Processor		*processor;
	OSCore			*oscore;
	Memory			*memory;
	IntController	*intcontroller;
	//MMU				*mem;
	//FS				*fileSystem;

public:
	System();
	~System();

	void start();
};

