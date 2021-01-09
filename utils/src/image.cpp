#include <assert.h>
#include <algorithm>
#include <string.h> //for memcpy
#include "FreeImage.h"
#include "image.h"
#include "threadman.h"
#include "timer.h"

typedef unsigned long long uint64;


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
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(fileName.c_str());
	FIBITMAP *dib = FreeImage_Load(fif, fileName.c_str());
	BYTE *bits    = FreeImage_GetBits(dib);
	valid = 0;
	if (!dib || !bits) {
		FreeImage_Unload(dib);
		return -1;
	}

	BITMAPINFO *info = FreeImage_GetInfo(dib);

	unsigned bpp = info->bmiHeader.biBitCount;
	height       = info->bmiHeader.biHeight;
	width        = info->bmiHeader.biWidth;
	data = new Color[width*height];
	colorSize = sizeof(Color);
	if (bpp == 96) {
		for (int i = 0; i < height; i++) {
			BYTE* line = FreeImage_GetScanLine(dib, i);
			FIRGBF * c = (FIRGBF*) line;
			for (int j = 0; j < width; j++) {
				Color col(c->red, c->green, c->blue);
				
				const float intens = col.intensity();
				if (minValue.intensity() > intens) {
					minValue = col;
				}
				if (maxValue.intensity() < intens) {
					maxValue = col;
				}
				data[i*width + j] = col;
				c++;
			}
		}
	} else if (bpp == 24) {
		for (int i = 0; i < height; i++) {
			BYTE* line = FreeImage_GetScanLine(dib, i);
			RGBTRIPLE * c = (RGBTRIPLE*) line;
			for (int j = 0; j < width; j++) {
				float r = c->rgbtRed   * scalar;
				float g = c->rgbtGreen * scalar;
				float b = c->rgbtBlue  * scalar;
				data[i*width + j] = Color(r, g, b);
				c++;
			}
		}
	} else if (bpp == 32) {
		for (int i = 0; i < height; i++) {
			BYTE* line = FreeImage_GetScanLine(dib, i);
			RGBQUAD * c = (RGBQUAD*) line;
			for (int j = 0; j < width; j++) {
				float r = c->rgbRed   * scalar;
				float g = c->rgbGreen * scalar;
				float b = c->rgbBlue  * scalar;
				data[i*width + j] = Color(r, g, b);
				c++;
			}
		}
	} else if (bpp == 8) {

		for (int i = 0; i < height; i++) {
			BYTE* line = FreeImage_GetScanLine(dib, i);
			for (int j = 0; j < width; j++) {
				float r = (*line) * scalar;
				float g = (*line) * scalar;
				float b = (*line) * scalar;
				data[i*width + j] = Color(r, g, b);
				line++;
			}
		}
	} else {
		FreeImage_Unload(dib);
		return 1;
	}
	FreeImage_Unload(dib);
	valid = 1;
	return 0;
}

struct B : a7az0th::MultiThreadedFor {
	B(const char* data, RGBTRIPLE *pixel_data, int width, int height, int jobSize, int stride): buff(data), pixel_data(pixel_data), width(width), height(height), stride(stride), jobSize(jobSize) {
	}
	virtual void body(int index, int threadIdx, int numThreads) override {
		for (int a = 0; a < jobSize; a++) {
			const int lineIdx = index*jobSize + a;
			if (lineIdx >= height) {
				return;
			}

			for (int i = 0; i < width; i++) {
				const int offset = lineIdx*width+i;
				const Color& col = *reinterpret_cast<const Color*>(buff + offset*stride);
				pixel_data[offset].rgbtRed   = static_cast<BYTE>(clamp(col.f.f[0], 0.0f, 1.0f) * 255.f);
				pixel_data[offset].rgbtGreen = static_cast<BYTE>(clamp(col.f.f[1], 0.0f, 1.0f) * 255.f);
				pixel_data[offset].rgbtBlue  = static_cast<BYTE>(clamp(col.f.f[2], 0.0f, 1.0f) * 255.f);
			}
		}
	}
private:
	int jobSize;
	int width;
	int height;
	int stride;
	RGBTRIPLE *pixel_data;
	const char *buff;
};

int Image::save(const std::string &fileName, a7az0th::ThreadManager *threadman) const {
	FIBITMAP *dib = FreeImage_Allocate(width, height, 24);
	RGBTRIPLE *pixel_data = (RGBTRIPLE*)FreeImage_GetBits(dib);
	if (!dib || !pixel_data) {
		FreeImage_Unload(dib);
		return -1;
	}
	const char* buffStart = reinterpret_cast<char*>(data);

	a7az0th::Timer elapsed;
	if (threadman) {

		const int numThreads = a7az0th::getProcessorCount();
		const int jobSize = (height + numThreads - 1) / numThreads;
		B b(buffStart, pixel_data, width, height, jobSize, colorSize);
		b.run(*threadman, jobSize, numThreads);
	} else {
		for (int i = 0; i < height*width; i++) {
			const Color& col = *reinterpret_cast<const Color*>(buffStart + i*colorSize);
			pixel_data->rgbtRed   = static_cast<BYTE>(clamp(col.f.f[0], 0.0f, 1.0f) * 255.f);
			pixel_data->rgbtGreen = static_cast<BYTE>(clamp(col.f.f[1], 0.0f, 1.0f) * 255.f);
			pixel_data->rgbtBlue  = static_cast<BYTE>(clamp(col.f.f[2], 0.0f, 1.0f) * 255.f);
			pixel_data++;
		}
	}
	a7az0th::int64 elapsedTime = elapsed.elapsed(a7az0th::Timer::Precision::Milliseconds);
	printf("Image conversion took %lld ms\n", elapsedTime);

	FreeImage_Save(FIF_JPEG, dib, fileName.c_str());
	FreeImage_Unload(dib);
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
