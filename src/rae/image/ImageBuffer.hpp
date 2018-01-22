#pragma once

#include "nanovg.h"

#include "rae/core/Types.hpp"

namespace rae
{

struct ImageBuffer
{
	ImageBuffer();
	ImageBuffer(int setWidth, int setHeight);

	void init(int setWidth, int setHeight);
	void init();

	void load(NVGcontext* vg, String file);
	void writeToPng(String filename);

	void createImage(NVGcontext* vg);
	void update8BitImageBuffer(NVGcontext* vg);
	void clear();

	void update(NVGcontext* vg);

	int channels = 4; // needs to be 4 for rgba with nanovg create image func
	
	int width;
	int height;

	Array<vec3> colorData;
	Array<uint8_t> data;

	int imageId;
	bool needsUpdate = false; // When set to true, the image will be created (if needed) and updated to nanovg.
};

}
