#include <iostream>
#include <ciso646>

#include "rae/core/Utils.hpp"
#include "rae/image/ImageBuffer.hpp"

using namespace rae;

vec3 pow(const vec3& color, float power)
{
	vec3 result;
	result.r = glm::pow(color.r, power);
	result.g = glm::pow(color.g, power);
	result.b = glm::pow(color.b, power);
	return result;
}

vec3 gammaCorrectionAnd255(const vec3& linear)
{
	const float gammaMul = 1.0f/2.2f;
	return 255.99f * glm::clamp( pow(linear, gammaMul), 0.0f, 1.0f);
}

//------------------------------------------------------------------------------------------------------------

ImageBuffer::ImageBuffer()
: width(0),
height(0),
imageId(-1) // init to invalid value.
{
}

ImageBuffer::ImageBuffer(int setWidth, int setHeight)
: width(setWidth),
height(setHeight),
imageId(-1) // init to invalid value.
{
	init();
}

void ImageBuffer::init(int setWidth, int setHeight)
{
	width = setWidth;
	height = setHeight;

	init();
}

void ImageBuffer::init()
{
	if (width == 0 || height == 0)
	{
		assert(0);
		return;
	}

	if (colorData.size() > 0)
		colorData.clear();
	if (data.size() > 0)
		data.clear();

	colorData.reserve(width * height);
	for (int i = 0; i < width * height; ++i)
	{
		colorData.push_back(vec3(0.5f, 0.5f, 0.5f));
	}	
	data.reserve(width * height * channels);
	for (int i = 0; i < width * height; ++i)
	{
		data.push_back(0);
		data.push_back(0);
		data.push_back(0);
		data.push_back(255);
	}
}

void ImageBuffer::load(NVGcontext* vg, String file)
{
	if(imageId == -1)
	{
		imageId = nvgCreateImage(vg, file.c_str(), 0);
	}
}

void ImageBuffer::createImage(NVGcontext* vg)
{
	if (imageId == -1 && vg != nullptr)
	{
		//std::cout << "Creating image " << width << "x" << height << "\n";
		imageId = nvgCreateImageRGBA(vg, width, height, /*imageFlags*/0, &data[0]);
	}
	else
	{
		std::cout << "Failed to create an image " << width << "x" << height << "\n";
		if (imageId != -1)
			std::cout << "imageId was not -1. It was " << imageId << "\n";
		if (vg == nullptr)
			std::cout << "vg was null.\n";
		assert(imageId == -1);
		assert(vg != nullptr);
	}
}

void ImageBuffer::clear()
{
	std::fill(colorData.begin(), colorData.end(), vec3(0,0,0));
	//std::fill(data.begin(), data.end(), 0);
}

void ImageBuffer::update(NVGcontext* vg)
{
	if (not needsUpdate)
		return;

	if (imageId == -1)
		createImage(vg);

	needsUpdate = false;
	nvgUpdateImage(vg, imageId, &data[0]);	
}

void ImageBuffer::update8BitImageBuffer(NVGcontext* vg)
{
	// update 8 bit image buffer
	{
		parallel_for(0, height, [&](int j)
		{
			for (int i = 0; i < width; ++i)
			{
				const vec3& linear = colorData[(j*width)+i];

				vec3 color = gammaCorrectionAnd255(linear);

				data[(j*width*channels) + (i*channels) + 0] = int8_t(color.r);
				data[(j*width*channels) + (i*channels) + 1] = int8_t(color.g);
				data[(j*width*channels) + (i*channels) + 2] = int8_t(color.b);
			}
		});

		nvgUpdateImage(vg, imageId, &data[0]);
	}
}
