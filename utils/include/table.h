#pragma once

#include <vector>

template<class T>
struct Table {
public:

	Table() { setCount(0); }
	Table(int size) { setCount(size); }

	~Table() {}

	T& operator[](int idx) { return container[idx]; }
	const T& operator[](int idx) const { return container[idx]; }

	T& operator+=(const T& rhs){
		container.push_back(rhs);
		return *container.rbegin();
	}

	void reserve(int size) {
		container.reserve(size);
	}

	void setCount(int size) {
		container.resize(size);
	}

	int count() const {
		return static_cast<int>(container.size());
	}

	void setAll(const T& defaultValue) {
		for (T& v : *this) {
			v = defaultValue;
		}
	}

	Table(const Table& rhs) = delete;
	Table& operator=(const Table& rhs) = delete;

	//write definition for begin() and end()
	//these two method will be used for "ranged based loop idiom"
	const T* begin() const { return &container[0]; }
	const T* end() const { return &container[0] + container.size(); }

	T* begin() { return &container[0]; }
	T* end() { return &container[0] + container.size(); }

	int find(const T &value) const {
		for (int i = 0; i < container.size(); i++) {
			if (container[i] == value) {
				return i;
			}
		}
		return -1;
	}

	void eraseKeepOrder(int idx) {
		container.erase(container.begin() + idx);
	}

	void copy(const Table<T> &rhs) {
		//container = rhs.container;
		container.resize(rhs.container.size());
		for (int i = 0; i < rhs.container.size(); i++) {
			container[i] = rhs.container[i];
		}
	}
private:
	std::vector<T> container;
};
