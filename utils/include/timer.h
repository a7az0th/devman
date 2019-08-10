#pragma once

#include <chrono>
#include <cassert>

namespace a7az0th {

typedef long long int int64 ;

// Class providing basic time measurement functionality
class Timer {
public:
	// Specify in what precision the timing should be
	enum class Precision {
		Seconds = 0,
		Milliseconds,///< 1/1000 of a second
		Microseconds,///< 1/1000 of a millisecond
		Nanoseconds, ///< 1/1000 of a microsecond
	};

	//Create a timer and start it
	Timer() { init(); }
	//Destoy a timer
	~Timer() {}

	//Restarts the timer
	void restart() { init(); }

	// Returns the elapsed time in the precision specified since either the timer was created or last restarted.
	// whichever occurred sooner
	int64 elapsed(Precision prec) const {
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
		case Precision::Microseconds: {
			const std::chrono::microseconds& total_ms = std::chrono::duration_cast<std::chrono::microseconds>(endPoint - startPoint);
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
	void read() const { endPoint = std::chrono::system_clock::now(); }
	void init() const { startPoint = endPoint = std::chrono::system_clock::now(); }
	// Disallow evil contructors
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	mutable std::chrono::system_clock::time_point startPoint;
	mutable std::chrono::system_clock::time_point endPoint;
};

template<int BufferLength>
struct FPSTimer {
	FPSTimer() : framesRendered(0), lastFrameTime(0), totalTime(0), summedHistory(0) {
		history.fill(0);
	}
	~FPSTimer() {}

	void start() {
		t.restart();
	}

	void stop() {
		lastFrameTime = t.elapsed(Timer::Precision::Microseconds);
		int64& historyElement = history[framesRendered % BufferLength];
		summedHistory = summedHistory - historyElement + lastFrameTime;
		historyElement = lastFrameTime;
		totalTime += lastFrameTime;
		framesRendered++;
	}

#if 1
	float getTemporalFPS() const {
		float res = 0.f;
		if (summedHistory > 1e-6f) {
			const int div = (framesRendered < BufferLength) ? framesRendered : BufferLength;
			res = float((1000000.0*div) / summedHistory);
		}
		return res;
	}
	float getMomentaryFPS() const {
		float res = 0.f;
		if (lastFrameTime > 1e-6f) {
			res = float(1000000.0 / lastFrameTime);
		}
		return res;
	}

	float getFPS() const {
		float res = 0.f;
		if (totalTime > 1e-6f) {
			res = float((1000000.0*framesRendered) / totalTime);
		}
		return res;
	}
#else
	int getTemporalFPS() const {
		int res = 0;
		if (summedHistory > 1e-6f) {
			const int div = (framesRendered < BufferLength) ? framesRendered : BufferLength;
			res = summedHistory / div;
		}
		return res;
	}
	int getMomentaryFPS() const {
		int res = 0;
		if (lastFrameTime > 1e-6f) {
			res = lastFrameTime / 1;
		}
		return res;
	}

	int getFPS() const {
		int res = 0;
		if (totalTime > 1e-6f) {
			res = totalTime / framesRendered;
		}
		return res;
	}
#endif

	int64 sinceLastRestart() const {
		return t.elapsed(Timer::Precision::Milliseconds);
	}

private:
	std::array<int64, BufferLength> history;
	int64 summedHistory;
	int framesRendered;
	int64 totalTime;
	int64 lastFrameTime;
	Timer t;
};

}
