// Running the program will rasterize a single triangle, and write it to `out.ppm`
// You can convert this file into a more common format like png using something like imagemagick.

// note: this program only supports Ubuntu OS
#include <cstring>
#include <cmath>
#include <cfloat>
#include <chrono>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <bitset>

// get SSE and AVX
#include <x86intrin.h>
#define EARLY_EXIT 0
#define SAVE_PIC 0
struct vec2 {
	public:
	float x;
	float y;

	vec2() : x(0.0f), y(0.0f) {}
	vec2(const float x_, const float y_) : x(x_), y(y_) {}
};

unsigned int rounddownAligned(unsigned int i, unsigned int align) {
	return (unsigned int)floor((float)i / (float)align) * align;
}

unsigned int roundupAligned(unsigned int i, unsigned int align) {
	return (unsigned int)ceil((float)i / (float)align) * align;
}

__m128 edgeFunctionSSE(const vec2& a, const vec2& b, __m128 cx, __m128 cy) {
	return _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(cx, _mm_set1_ps(a.x)), _mm_set1_ps(b.y - a.y)), _mm_mul_ps(_mm_sub_ps(cy, _mm_set1_ps(a.y)), _mm_set1_ps(b.x - a.x)));
}

__m256 edgeFunctionAVX(const vec2& a, const vec2& b, __m256 cx, __m256 cy) {
	return _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(cx, _mm256_set1_ps(a.x)), _mm256_set1_ps(b.y - a.y)), _mm256_mul_ps(_mm256_sub_ps(cy, _mm256_set1_ps(a.y)), _mm256_set1_ps(b.x - a.x)));
}

float edgeFunction(const vec2& a, const vec2& b, const vec2& c) {
	// we need to do it in this way, since our coordinate system has the origin in the top-left corner.
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void print_m128_bits(const __m128 var) {
	float f[4]; // 用于存储从 __m128 中提取的浮点数
	_mm_storeu_ps(f, var); // 将 __m128 变量存储到普通的 float 数组

	// 遍历数组，并打印每个 float 的位
	for (int i = 0; i < 4; i++) {
		unsigned int* asInt = reinterpret_cast<unsigned int*>(&f[i]);
		std::bitset<32> binary(*asInt);
		std::cout << binary << std::endl;
	}
}

void rasterizeTriangleCacheUnfriendly(
	const float x0, const float y0,
	const float x1, const float y1,
	const float x2, const float y2,
	const unsigned int fbWidth, const unsigned int fbHeight,
	unsigned int* framebuffer
) {
	vec2 vcoords[3];
	vcoords[0] = vec2(x0, y0);
	vcoords[1] = vec2(x1, y1);
	vcoords[2] = vec2(x2, y2);

	// min and max of triangle AABB
	vec2 amin(+FLT_MAX, +FLT_MAX);
	vec2 amax(-FLT_MAX, -FLT_MAX);

	// find triangle AABB.
	for (int i = 0; i < 3; ++i) {
		vec2 p = vcoords[i];

		amin.x = std::min(p.x, amin.x);
		amin.y = std::min(p.y, amin.y);

		amax.x = std::max(p.x, amax.x);
		amax.y = std::max(p.y, amax.y);
	}

	// make sure we don't rasterize outside the framebuffer.
	// range from [-1.0, 1.0]
	amin.x = std::clamp(amin.x, -1.0f, +1.0f);
	amax.x = std::clamp(amax.x, -1.0f, +1.0f);
	amin.y = std::clamp(amin.y, -1.0f, +1.0f);
	amax.y = std::clamp(amax.y, -1.0f, +1.0f);

	// scale [-1.0, 1.0], [-1.0, 1.0] to [0, fbWidth], [0, fbHeight]
	// and round up/down by `align` units for the SIMD memory access
	int align = 1;
	const float intAminx = static_cast<float>(rounddownAligned(static_cast<unsigned int>((0.5f + 0.5f * amin.x) * fbWidth), align));
	const float intAmaxx = static_cast<float>(roundupAligned(static_cast<unsigned int>((0.5f + 0.5f * amax.x) * fbWidth), align));
	const float intAminy = float((0.5f + 0.5f * amin.y) * fbHeight);
	const float intAmaxy = float((0.5f + 0.5f * amax.y) * fbHeight);

	// each pixel's size
	const float doublePixelWidth = 2.0f / (float)fbWidth;
	const float doublePixelHeight = 2.0f / (float)fbHeight;

	vec2 p;

	for (float iy = intAminy; iy <= (float)intAmaxy; iy += 1.0f) {
		// map from [0,height] to [-1, +1]
		p.y = -1.0f + iy * doublePixelHeight;
		for (float ix = intAminx; ix <= (float)intAmaxx; ix += 1.0f) {
			// map from [0,width] to [-1, +1]
			p.x = -1.0f + ix * doublePixelWidth;

			if (EARLY_EXIT) {
				// early exit if any of the edges is outside the triangle
				float w0 = edgeFunction(vcoords[1], vcoords[2], p);
				if (w0 < 0.0f) break;
				float w1 = edgeFunction(vcoords[2], vcoords[0], p);
				if (w1 < 0.0f) break;
				float w2 = edgeFunction(vcoords[0], vcoords[1], p);
				if (w2 < 0.0f) break;
				unsigned int iBuf = static_cast<unsigned int>(iy * fbWidth + ix);
				framebuffer[iBuf] = 0xFFFFFFFF;
			}
			else {
				// normal exit
				float w0 = edgeFunction(vcoords[1], vcoords[2], p);
				float w1 = edgeFunction(vcoords[2], vcoords[0], p);
				float w2 = edgeFunction(vcoords[0], vcoords[1], p);
				if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
					unsigned int iBuf = (unsigned int)(iy * fbWidth + ix);
					framebuffer[iBuf] = 0xFFFFFFFF;
				}
			}
		}
	}
}

void rasterizeTriangleCacheFriendly(
	const float x0, const float y0,
	const float x1, const float y1,
	const float x2, const float y2,
	const unsigned int fbWidth, const unsigned int fbHeight,
	unsigned int* framebuffer
) {
	vec2 vcoords[3];
	vcoords[0] = vec2(x0, y0);
	vcoords[1] = vec2(x1, y1);
	vcoords[2] = vec2(x2, y2);

	// min and max of triangle AABB
	vec2 amin(+FLT_MAX, +FLT_MAX);
	vec2 amax(-FLT_MAX, -FLT_MAX);

	// find triangle AABB.
	for (int i = 0; i < 3; ++i) {
		vec2 p = vcoords[i];

		amin.x = std::min(p.x, amin.x);
		amin.y = std::min(p.y, amin.y);

		amax.x = std::max(p.x, amax.x);
		amax.y = std::max(p.y, amax.y);
	}

	// make sure we don't rasterize outside the framebuffer.
	// range from [-1.0, 1.0]
	amin.x = std::clamp(amin.x, -1.0f, +1.0f);
	amax.x = std::clamp(amax.x, -1.0f, +1.0f);
	amin.y = std::clamp(amin.y, -1.0f, +1.0f);
	amax.y = std::clamp(amax.y, -1.0f, +1.0f);

	// scale [-1.0, 1.0], [-1.0, 1.0] to [0, fbWidth], [0, fbHeight]
	// and round up/down by `align` units for the SIMD memory access
	int align = 1;
	const float intAminy = static_cast<float>(rounddownAligned(static_cast<unsigned int>((0.5f + 0.5f * amin.y) * fbWidth), align));
	const float intAmaxy = static_cast<float>(roundupAligned(static_cast<unsigned int>((0.5f + 0.5f * amax.y) * fbWidth), align));
	const float intAminx = float((0.5f + 0.5f * amin.x) * fbHeight);
	const float intAmaxx = float((0.5f + 0.5f * amax.x) * fbHeight);

	// each pixel's size
	const float doublePixelWidth = 2.0f / (float)fbWidth;
	const float doublePixelHeight = 2.0f / (float)fbHeight;

	vec2 p;

	for (float ix = intAminx; ix <= (float)intAmaxx; ix += 1.0f) {
		// map from [0,width] to [-1, +1]
		p.x = -1.0f + ix * doublePixelWidth;
		for (float iy = intAminy; iy <= (float)intAmaxy; iy += 1.0f) {
			// map from [0,height] to [-1, +1]
			p.y = -1.0f + iy * doublePixelHeight;

			if (EARLY_EXIT) {
				// early exit if any of the edges is outside the triangle
				float w0 = edgeFunction(vcoords[1], vcoords[2], p);
				if (w0 < 0.0f) break;
				float w1 = edgeFunction(vcoords[2], vcoords[0], p);
				if (w1 < 0.0f) break;
				float w2 = edgeFunction(vcoords[0], vcoords[1], p);
				if (w2 < 0.0f) break;
				unsigned int iBuf = static_cast<unsigned int>(ix * fbHeight + iy);
				framebuffer[iBuf] = 0xFFFFFFFF;
			}
			else {
				// normal exit
				float w0 = edgeFunction(vcoords[1], vcoords[2], p);
				float w1 = edgeFunction(vcoords[2], vcoords[0], p);
				float w2 = edgeFunction(vcoords[0], vcoords[1], p);
				if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
					unsigned int iBuf = static_cast<unsigned int>(ix * fbHeight + iy);
					framebuffer[iBuf] = 0xFFFFFFFF;
				}
			}
		}
	}
}

void rasterizeTriangleSSE(
	const float x0, const float y0,
	const float x1, const float y1,
	const float x2, const float y2,
	const unsigned int fbWidth,
	const unsigned int fbHeight,
	unsigned int* framebuffer) {
	vec2 vcoords[3];
	vcoords[0] = vec2(x0, y0);
	vcoords[1] = vec2(x1, y1);
	vcoords[2] = vec2(x2, y2);

	// min and max of triangle AABB
	vec2 amin(+FLT_MAX, +FLT_MAX);
	vec2 amax(-FLT_MAX, -FLT_MAX);

	// find triangle AABB.
	for (int i = 0; i < 3; ++i) {
		vec2 p = vcoords[i];
		amin.x = std::min(p.x, amin.x);
		amin.y = std::min(p.y, amin.y);
		amax.x = std::max(p.x, amax.x);
		amax.y = std::max(p.y, amax.y);
	}

	// make sure we don't rasterize outside the framebuffer..
	amin.x = std::clamp(amin.x, -1.0f, +1.0f);
	amax.x = std::clamp(amax.x, -1.0f, +1.0f);
	amin.y = std::clamp(amin.y, -1.0f, +1.0f);
	amax.y = std::clamp(amax.y, -1.0f, +1.0f);

	// craete a float whose all bits are set to 1.
	float filledbitsfloat;
	unsigned int ii = 0xffffffff;
	memcpy(&filledbitsfloat, &ii, sizeof(float));
	float whitecolorfloat = filledbitsfloat;

	/*
	We'll be looping over all pixels in the AABB, and rasterize the pixels within the triangle. The AABB has been
	extruded on the x-axis, and aligned to 16bytes.
	This is necessary since _mm_store_ps can only write to 16-byte aligned addresses.
	*/
	const float intAminx = (float)rounddownAligned(int((0.5f + 0.5f * amin.x) * fbWidth), 4); // 4 pixels once
	const float intAmaxx = (float)roundupAligned(int((0.5f + 0.5f * amax.x) * fbWidth), 4);
	const float intAminy = (float)int((0.5f + 0.5f * amin.y) * fbHeight);
	const float intAmaxy = (float)int((0.5f + 0.5f * amax.y) * fbHeight);

	const float doublePixelWidth = 2.0f / (float)fbWidth;
	const float doublePixelHeight = 2.0f / (float)fbHeight;

	__m128 minusone = _mm_set1_ps(-1.0f);
	__m128 zero = _mm_setzero_ps();

	for (float iy = intAminy; iy <= (float)intAmaxy; iy += 1.0f) {
		// map from [0,height] to [-1,+1]
		__m128 py = _mm_add_ps(minusone, _mm_mul_ps(_mm_set1_ps(iy), _mm_set1_ps(doublePixelHeight)));

		for (float ix = intAminx; ix <= (float)intAmaxx; ix += 4.0f) {
			// this `px` register contains the x-coords of four pixels in a row.
			// we map from [0,width] to [-1,+1]
			// -1 + x * doublePixelWidth
			__m128 px = _mm_add_ps(minusone, _mm_mul_ps(_mm_set_ps(ix + 3.0f, ix + 2.0f, ix + 1.0f, ix + 0.0f), _mm_set1_ps(doublePixelWidth)));

			// the default bitflag, results in all the four pixels being overwritten.
			__m128 writeFlag = _mm_set1_ps(filledbitsfloat);
			
			__m128 w0 = edgeFunctionSSE(vcoords[1], vcoords[2], px, py);
			__m128 w1 = edgeFunctionSSE(vcoords[2], vcoords[0], px, py);
			__m128 w2 = edgeFunctionSSE(vcoords[0], vcoords[1], px, py);

			// the results of the edge tests are used to modify our bitflag.
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w0, zero));
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w1, zero));
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w2, zero));

			unsigned int iBuf = static_cast<unsigned int>(iy * fbWidth + ix);
			__m128 newBufferVal = _mm_set1_ps(whitecolorfloat);
			__m128 origBufferVal = _mm_loadu_ps((const float*)framebuffer + iBuf);

			/*
			We only want to write to pixels that are inside the triangle.
			However, implementing such a conditional write is tricky when dealing SIMD.

			We implement this by using a bitflag. This bitflag determines which of the four floats in __m128 should
			just write the old value to the buffer(meaning that the pixel is NOT actually rasterized),
			and which should overwrite the current value in the buffer(meaning that the pixel IS rasterized).

			This is implemented by some bitwise manipulation tricks.
			*/
			_mm_storeu_ps((float*)(framebuffer)+iBuf,
				_mm_or_ps(_mm_and_ps(writeFlag, newBufferVal), _mm_andnot_ps(writeFlag, origBufferVal)));
		}
	}
}

void rasterizeTriangleAVX(
	const float x0, const float y0,
	const float x1, const float y1,
	const float x2, const float y2,
	const unsigned int fbWidth,
	const unsigned int fbHeight,
	unsigned int* framebuffer) {
	vec2 vcoords[3];
	vcoords[0] = vec2(x0, y0);
	vcoords[1] = vec2(x1, y1);
	vcoords[2] = vec2(x2, y2);

	// min and min of triangle AABB
	vec2 amin(+FLT_MAX, +FLT_MAX);
	vec2 amax(-FLT_MAX, -FLT_MAX);

	// find triangle AABB.
	for (int i = 0; i < 3; ++i) {
		vec2 p = vcoords[i];
		amin.x = std::min(p.x, amin.x);
		amin.y = std::min(p.y, amin.y);
		amax.x = std::max(p.x, amax.x);
		amax.y = std::max(p.y, amax.y);
	}

	// make sure we don't rasterize outside the framebuffer..
	amin.x = std::clamp(amin.x, -1.0f, +1.0f);
	amax.x = std::clamp(amax.x, -1.0f, +1.0f);
	amin.y = std::clamp(amin.y, -1.0f, +1.0f);
	amax.y = std::clamp(amax.y, -1.0f, +1.0f);

	// float where are all bits are set.
	float filledbitsfloat;
	unsigned int ii = 0xffffffff;
	memcpy(&filledbitsfloat, &ii, sizeof(float));
	float whitecolorfloat = filledbitsfloat;

	const float intAminx = (float)rounddownAligned(int((0.5f + 0.5f * amin.x) * fbWidth), 8); // 8 pixels once
	const float intAmaxx = (float)roundupAligned(int((0.5f + 0.5f * amax.x) * fbWidth), 8);
	const float intAminy = (float)((0.5f + 0.5f * amin.y) * fbHeight);
	const float intAmaxy = (float)((0.5f + 0.5f * amax.y) * fbHeight);

	const float doublePixelWidth = 2.0f / (float)fbWidth;
	const float doublePixelHeight = 2.0f / (float)fbHeight;

	__m256 minusone = _mm256_set1_ps(-1.0f);
	__m256 zero = _mm256_setzero_ps();

	for (float iy = intAminy; iy <= intAmaxy; iy += 1.0f) {
		// map from [0,height] to [-1,+1]
		__m256 py = _mm256_add_ps(minusone, _mm256_mul_ps(_mm256_set1_ps(iy), _mm256_set1_ps(doublePixelHeight)));

		for (float ix = intAminx; ix <= intAmaxx; ix += 8.0f) {
			// we map from [0,width] to [-1,+1]
			__m256 px = _mm256_add_ps(minusone, _mm256_mul_ps(
				_mm256_set_ps(ix + 7.0f, ix + 6.0f, ix + 5.0f, ix + 4.0f, ix + 3.0f, ix + 2.0f, ix + 1.0f, ix + 0.0f), _mm256_set1_ps(doublePixelWidth)));

			__m256 w0 = edgeFunctionAVX(vcoords[1], vcoords[2], px, py);
			__m256 w1 = edgeFunctionAVX(vcoords[2], vcoords[0], px, py);
			__m256 w2 = edgeFunctionAVX(vcoords[0], vcoords[1], px, py);

			__m256 writeFlag = _mm256_set1_ps(filledbitsfloat);

			// the results of the edge tests are used to modify our bitflag.
			writeFlag = _mm256_and_ps(writeFlag, _mm256_cmp_ps(w0, zero, _CMP_NLT_US));
			writeFlag = _mm256_and_ps(writeFlag, _mm256_cmp_ps(w1, zero, _CMP_NLT_US));
			writeFlag = _mm256_and_ps(writeFlag, _mm256_cmp_ps(w2, zero, _CMP_NLT_US));

			unsigned int iBuf = static_cast<unsigned int>(iy * fbWidth + ix);

			__m256 newBufferVal = _mm256_set1_ps(whitecolorfloat);
			// __m256 origBufferVal = _mm256_load_ps((const float*)framebuffer + iBuf);
			__m256 origBufferVal = _mm256_loadu_ps((const float*)framebuffer + iBuf);

			
			_mm256_storeu_ps((float*)(framebuffer)+iBuf,
				_mm256_or_ps(_mm256_and_ps(writeFlag, newBufferVal), _mm256_andnot_ps(writeFlag, origBufferVal)));
		}
	}
}


int main(int argc, char* argv[]) {
	// const unsigned int UNIT_LENGTH = 50;
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <UNIT_LENGTH>" << std::endl;
		return 1;
	}
	const unsigned int UNIT_LENGTH = std::atoi(argv[1]);
	const unsigned int WIDTH = 3*UNIT_LENGTH;
	const unsigned int HEIGHT = 2*UNIT_LENGTH;

	auto framebuffer = std::make_unique<unsigned int[]>(WIDTH * HEIGHT); // 32 bits per pixel

	if (!framebuffer) {
		printf("Memory allocation failed\n");
		return 1;
	}

	// clear framebuffer
	for (size_t row = 0; row < HEIGHT; ++row) {
		for (size_t col = 0; col < WIDTH; ++col) {
			framebuffer[row * WIDTH + col] = (255 << 0) | (0 << 8) | (0 << 16); // R = 255, B = 0, G = 0
		}
	}

	// triangle vertices. in range [-1.0, +1.0)
	float x0 = 1.0f; float y0 = 0.9999f;
	float x1 = 0.999f; float y1 = -1.0f;
	float x2 = -1.0f; float y2 = -0.65f;

	auto runRasterize = [&](auto func, const char* name) {
		auto start = std::chrono::high_resolution_clock::now();
		func(x0, y0, x1, y1, x2, y2, WIDTH, HEIGHT, framebuffer.get());
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cout << name << " took " << elapsed.count() << " ms" << std::endl;
	};

	// Run all rasterization modes
	std::cout << "Rasterizing using Cache-friendly mode." << std::endl;
	runRasterize(rasterizeTriangleCacheFriendly, "Cache-friendly Mode");

	std::cout << "Rasterizing using Cache-unfriendly mode." << std::endl;
	runRasterize(rasterizeTriangleCacheUnfriendly, "Cache-unfriendly Mode");

	std::cout << "Rasterizing using SSE mode." << std::endl;
	runRasterize(rasterizeTriangleSSE, "SSE Mode");

	std::cout << "Rasterizing using AVX mode." << std::endl;
	runRasterize(rasterizeTriangleAVX, "AVX Mode");

	// output the rasterized triangle to `out.ppm`
	if (SAVE_PIC) {
		std::ofstream fp("out.ppm");
		if (!fp) {
			std::cerr << "Could not open output file for writing!\n";
			return 1;
		}

		fp << "P3\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
		for (unsigned int row = 0; row < HEIGHT; ++row) {
			for (unsigned int col = 0; col < WIDTH; ++col) {
				unsigned int c = framebuffer[row * WIDTH + col];
				unsigned char r = 0xFF & (c >> 0);
				unsigned char g = 0xFF & (c >> 8);
				unsigned char b = 0xFF & (c >> 16);
				fp << static_cast<int>(r) << ' ' << static_cast<int>(g) << ' ' << static_cast<int>(b) << ' ';
			}
			fp << '\n';
		}
	}
}
