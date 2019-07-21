#include <stdio.h>
#include "devman.h"
#include "timer.h"
#include "progress.h"
#include "utils.h"

#include "threadman.h"
#include "image.h"

using namespace a7az0th;



int main() {
	const int numThreads = getProcessorCount();
	ThreadManager threadman;
	ProgressCallback progress;

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	progress.info("%d devices found", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device &d = devman.getDevice(i);
		std::string info = d.getInfo();
		progress.info("%s", info.c_str());
	}

	Image img("D:/elisaveta.JPG");

	Device &d = devman.getDevice(0);
	d.makeCurrent();
#ifdef WIN32
	d.setSource("D:/code/devman_full/devman/gpu_code/greyscale.ptx");
#else
	d.setSource("/home/a7az0th/code/devman/gpu_code/greyscale.ptx");
#endif
	DeviceBuffer buffIn;
	DeviceBuffer buffOut;

	const int64 buffSize = img.getMemUsage();
	buffIn.alloc(buffSize);
	buffOut.alloc(buffSize);

	ThreadData tData(d);

	//Remove the context from the stack. We no longer need it
	CUcontext ctx=0;
	cuCtxGetCurrent(&ctx);
	cuCtxPopCurrent(NULL);
	cuCtxGetCurrent(&ctx);


	Kernel kernelGlobal("greyscale", d.getProgram());
	kernelGlobal.addParamPtr(buffIn.get());
	kernelGlobal.addParamPtr(buffOut.get());

	Timer elapsedTimer;

	Color* data = img.getData();
	CUstream stream = tData.getStream();
	buffIn.uploadAsync(data, buffSize, stream);
	tData.launch(kernelGlobal, img.getWidth() * img.getHeight());
	buffOut.downloadAsync(data, stream);
	
	tData.wait();
	const int64 globalTime = elapsedTimer.elapsed(Timer::Precision::Milliseconds);
	progress.info("Global Kernel took %lld ms", globalTime);


	elapsedTimer.restart();
	img.save("output.png", nullptr);//&threadman
	const int64 saving = elapsedTimer.elapsed(Timer::Precision::Milliseconds);
	progress.info("Image save took %lld ms", saving);

	//Restore the context so deinitialization may pass
	d.makeCurrent();
	return 0;
}
