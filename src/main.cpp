#include <stdio.h>
#include "devman.h"
#include "timer.h"
#include "progress.h"
#include "utils.h"

#include "threadman.h"
#include "image.h"

using namespace a7az0th;

void resetContexts() {
	CUcontext ctx=0;
	do {
		cuCtxGetCurrent(&ctx);
		cuCtxPopCurrent(NULL);
		cuCtxGetCurrent(&ctx);
	} while (ctx);
}

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

	Image img("D:/Tamara-2.jpg");
	const int64 buffSize = img.getMemUsage();

#ifdef WIN32
	std::string ptxFile("D:/code/devman/gpu_code/greyscale.ptx");
#else
	std::string ptxFile("/home/a7az0th/code/devman/gpu_code/greyscale.ptx");
#endif

	struct DevData {
		DeviceBuffer buffIn;
		DeviceBuffer buffOut;
		ThreadData *tData;

		DevData() : tData(nullptr) {}
		~DevData() { delete tData; }
	};

	DevData deviceData[3];

	for (int i = 0; i < devman.getDeviceCount(); i++) {
		Device &d = devman.getDevice(i);
		DevData &devData = deviceData[i];

		d.makeCurrent();
		d.setSource(ptxFile);

		devData.buffIn.alloc(buffSize);
		devData.buffOut.alloc(buffSize);
		devData.tData = new ThreadData(d);
	}

	resetContexts();

	for (int i = 0; i < devman.getDeviceCount(); i++) {

		Device &d = devman.getDevice(i);
		DevData &devData = deviceData[i];
		ThreadData &tData = *devData.tData;

		Kernel kernelGlobal("greyscale", d.getProgram());
		kernelGlobal.addParamPtr(devData.buffIn.get());
		kernelGlobal.addParamPtr(devData.buffOut.get());

		Timer elapsedTimer;

		Color* data = img.getData();
		CUstream stream = tData.getStream();
		devData.buffIn.uploadAsync(data, buffSize, stream);
		tData.launch(kernelGlobal, img.getWidth() * img.getHeight());
		//buffOut.downloadAsync(data, stream);

		const int64 globalTime = elapsedTimer.elapsed(Timer::Precision::Milliseconds);
		progress.info("Global Kernel took %lld ms", globalTime);
	}

	for (int i = 0; i < devman.getDeviceCount(); i++) {
		DevData &devData = deviceData[i];
		ThreadData &tData = *devData.tData;
		tData.wait();
	}

	Timer elapsedTimer;
	img.save("output.png", nullptr);//&threadman
	const int64 saving = elapsedTimer.elapsed(Timer::Precision::Milliseconds);
	progress.info("Image save took %lld ms", saving);

	//Restore the context so deinitialization may pass
	//d.makeCurrent();
	return 0;
}
