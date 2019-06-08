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

	Kernel kernelGlobal("dummyGlobal", d.getProgram());
	kernelGlobal.addParamPtr(buffA.get());
	kernelGlobal.addParamPtr(buffB.get());
	kernelGlobal.addParamPtr(buffC.get());

	ThreadData tData(d);
	Timer t;
	tData.launch(kernelGlobal, WIDTH*WIDTH);
	tData.wait();
	const int64 globalTime = t.elapsed(Timer::Precision::Nanoseconds);
	progress.info("Global Kernel took %lld ns", globalTime);

	buffC.download(arr);

	for (int i = 0; i < 10; i++) {
		printf("%d ", arr[i]);
	}
	printf("\n");

	Kernel kernelShared("dummyShared", d.getProgram());
	kernelShared.addParamPtr(buffA.get());
	kernelShared.addParamPtr(buffB.get());
	kernelShared.addParamPtr(buffC.get());

	t.restart();
	tData.launch(kernelShared, WIDTH*WIDTH);
	tData.wait();
	const int64 sharedTime = t.elapsed(Timer::Precision::Nanoseconds);
	progress.info("Shared Kernel took %lld ns", sharedTime);

	buffC.download(arr);

	for (int i = 0; i < 10; i++) {
		printf("%d ", arr[i]);
	}
	printf("\n");

	progress.info("Shared kernel is %lf times faster", double(globalTime)/double(sharedTime));

	return 0;
}
