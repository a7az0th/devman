#include <stdio.h>
#include "devman.h"


using namespace a7az0th;

int main() {

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	printf("%d devices found\n", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device& d = devman.getDevice(i);
		std::string info = d.getInfo();
		printf("%s\n", info.c_str());
	}


	return 0;
}
