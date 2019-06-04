#include <stdio.h>
#include "devman.h"
#include "timer.h"
#include "progress.h"
#include "utils.h"

using namespace a7az0th;

int arr[WIDTH*WIDTH];

void initBuffer(DeviceBuffer &buff)
{
	buff.alloc(sizeof(int)*WIDTH*WIDTH);
	for (int i = 0; i < WIDTH*WIDTH; i++) {
		arr[i] = rand();
	}
	buff.upload(arr, sizeof(int)*WIDTH*WIDTH);
}

int main() {

	ProgressCallback progress;

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	progress.info("%d devices found", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device &d = devman.getDevice(i);
		std::string info = d.getInfo();
		progress.info("%s", info.c_str());
	}

	Device &d = devman.getDevice(0);
	d.makeCurrent();
	d.setSource("D:/code/devman_full/devman/gpu_code/kernel.ptx");

	DeviceBuffer buffA;
	DeviceBuffer buffB;
	DeviceBuffer buffC;
	initBuffer(buffA);
	initBuffer(buffB);
	initBuffer(buffC);

	Kernel kernel("dummy", d.getProgram());
	kernel.addParamPtr(buffA.get());
	kernel.addParamPtr(buffB.get());
	kernel.addParamPtr(buffC.get());

	ThreadData tData(d);
	Timer t;
	t.start();
	tData.launch(kernel, WIDTH*WIDTH);
	tData.wait();
	t.stop();
	const int elapsedNs = t.elapsed(Timer::Precision::Milliseconds);
	progress.info("Kernel took %d", elapsedNs);

	buffC.download(arr);
	return 0;
}
