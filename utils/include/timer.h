#pragma once

#include <chrono>
#include <cassert>

namespace a7az0th {

typedef long long int int64 ;

// Class providing basic time measurement functionality
// Stop must be explicitly called!
class Timer {
public:
	// Specify in what precision the timing should be
	enum class Precision {
		Seconds = 0,
		Milliseconds,
		Nanoseconds,
	};

	//Create a timer and start it
	Timer() { init(); }
	//Destoy a timer
	~Timer() {};

	//Restarts the timer
	void restart() { init(); }

	// Returns the elapsed time in the precision specified since either the timer was created or last restarted.
	// whichever occurred sooner
	int64 elapsed(Precision prec) {
		read();
		int64 val = 0;
		switch (prec) {
		case Precision::Seconds: {
			const std::chrono::seconds& total_s = std::chrono::duration_cast<std::chrono::seconds>(endPoint - startPoint);
			val = total_s.count();
			break;
		}
		case Precision::Milliseconds: {
			const std::chrono::milliseconds& total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(endPoint - startPoint);
			val = total_ms.count();
			break;
		}
		case Precision::Nanoseconds: {
			const std::chrono::nanoseconds& total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(endPoint - startPoint);
			val = total_ns.count();
			break;
		}
		default:
			assert(false);
		}
		return val;
	}

private:
	void read() { endPoint = std::chrono::system_clock::now(); }
	void init() {
		startPoint = endPoint = std::chrono::system_clock::now();
	}
	// Disallow evil contructors
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	std::chrono::system_clock::time_point startPoint;
	std::chrono::system_clock::time_point endPoint;
};

}
