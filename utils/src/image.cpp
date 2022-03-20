#include <assert.h>
#include <algorithm>
#include <string.h> //for memcpy
#include "image.h"
#include "threadman.h"
#include "timer.h"

typedef unsigned long long uint64;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

template<typename T>
inline T clamp(const T& val, const T& minVal, const T& maxVal) { return std::min(std::max(val, minVal), maxVal); }

#ifdef _WIN32
	static const char slashSymbol = '\\';
#else
	static const char slashSymbol = '/';
#endif

Image::~Image() {
	freeImage();
}

Image::Image() {
	data = NULL;
	width = height = 0;
	valid = 0;
	name = std::string("Unnamed");
	own = 1;
	minValue = Color(1e12f, 1e12f, 1e12f);
	maxValue = Color(0.f, 0.f, 0.f);
	colorSize = sizeof(Color);
}

Image::Image(const Image &rhs) {
	data = NULL;
	copyImage(rhs);
	own = 1;
}

Image::Image(const int width, const int height, const std::string &fName) {
	data = NULL;
	allocateImage(width, height);
	minValue = Color(1e12f, 1e12f, 1e12f);
	maxValue = Color(0.f, 0.f, 0.f);
	name = fName;
	colorSize = sizeof(Color);
	own = 1;
}

Image::Image(const int width, const int height, void* buffer, int sizeOfColor, const std::string &fName) {
	this->width = width;
	this->height = height;
	this->colorSize = sizeOfColor;
	data = (Color*)buffer;
	name = fName;
	valid = 1;
	own = 0;
	minValue = Color(1e12f, 1e12f, 1e12f);
	maxValue = Color(0.f, 0.f, 0.f);

}

Image::Image(const std::string & fileName) {
	size_t pos = fileName.find_last_of(slashSymbol);
	if (pos != std::string::npos) {
		name = fileName.substr(pos+1);
	}
	minValue = Color(1e12f, 1e12f, 1e12f);
	maxValue = Color(0.f, 0.f, 0.f);
	width = 0;
	height = 0;
	colorSize = 0;
	if (-1 == loadFromFile(fileName)) {
		valid = 0;
		own = 0;
		data = nullptr;
	} else {
		valid = 1;
		own = 1;
	}
}

inline void Image::freeImage() {
	if (own) {
		delete [] data;
	}
	data = NULL;
	valid = 0;
	width = height = 0;
	minValue = Color(1e12f, 1e12f, 1e12f);
	maxValue = Color(0.f, 0.f, 0.f);
}

inline void Image::copyImage(const Image &rhs) {
	if (this == &rhs) return;
	freeImage();
	valid = rhs.isValid();
	width  = rhs.getWidth();
	height = rhs.getHeight();
	minValue = rhs.getMinColor();
	maxValue = rhs.getMaxColor();
	assert(false && "Initialize colorSize");
	//colorSize = rhs.Gsizeof(Color);
	name = rhs.name;
	data = new Color[width*height];
	memcpy(data, rhs.getData(), sizeof(Color)*width*height);
}

inline int Image::allocateImage(const int newWidth, const int newHeight) {
	freeImage();
	data = new Color[newWidth*newHeight];
	if (!data) return -1;
	width  = newWidth;
	height = newHeight;
	valid = 1;
	return 0;
}

int Image::allocate(const int width, const int height) {
	return allocateImage(width, height);
}

Color Image::getPixel(const int x, const int y) const {
	int w = clamp(x, 0, width-1);
	int h = clamp(y, 0, height-1);
	return data[h*width+w];
}

Color Image::getPixel(const float u, const float v) const {
	int w = int(u * (width-1));
	int h = int(v * (height-1));
	w = clamp(w, 0, width-1);
	h = clamp(h, 0, height-1);
	return data[h*width+w];
}

Color* Image::getLine(const int idx) const {
	if (idx < 0 || idx >= height) return NULL;
	return &data[idx*width];
}

int Image::load(const std::string & fileName) {
	freeImage();
	return loadFromFile(fileName);
}

const float scalar = 1.f / 255.f;

int Image::loadFromFile(const std::string & fileName) {
	int channels;
	unsigned char *img = stbi_load(fileName.c_str(), &width, &height, &channels, 0);

	for (int i = 0; i < width*height; i++) {
		Color col;
		if (channels == 1) {
			col.f.f[0] = img[i];
		} else if (channels == 3) {
			col.f.f[0] = img[3*i+0];
			col.f.f[1] = img[3*i+1];
			col.f.f[2] = img[3*i+2];
		} else {
			assert(false);
		}
		data[i] = col;
	}

	stbi_image_free(img);  
	valid = 1;
	return 0;
}


int Image::save(const std::string &fileName, a7az0th::ThreadManager *threadman) const {

	return 0;
}

Color       Image::getMinColor() const              { return minValue; }
Color       Image::getMaxColor() const              { return maxValue; }
void        Image::rename(const std::string &fName) { name = fName; }
int         Image::getWidth()    const              { return width; }
int         Image::getHeight()   const              { return height; }
bool        Image::isValid()     const              { return valid; }
Color*      Image::getData()     const              { return data; }
uint64      Image::getMemUsage() const              { return width*height*sizeof(Color); }
std::string Image::getName()     const              { return name; }
