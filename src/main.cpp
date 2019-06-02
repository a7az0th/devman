#include <stdio.h>
#include "devman.h"
#include "table.h"

using namespace a7az0th;

int main() {

	DeviceManager &devman = DeviceManager::getInstance();
	const int numDevices = devman.getDeviceCount();
	printf("%d devices found\n", numDevices);

	for (int i = 0; i < numDevices; i++) {
		Device& d = devman.getDevice(i);
		std::string info = d.getInfo();

		DeviceBuffer buffer;
		buffer.alloc(1000000000);//allocate 1 GB

		printf("%s\n", info.c_str());
	}


	return 0;
}
