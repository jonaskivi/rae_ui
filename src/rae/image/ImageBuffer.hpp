#pragma once

#include "nanovg.h"

#include <array>

#include "rae/core/Types.hpp"

namespace rae
{

enum Channel
{
	R,
	G,
	B,
	A
};

using Pixel8_t = std::array<uint8_t, 4>;

class ImageBuffer
{
public:
	ImageBuffer();
	ImageBuffer(int width, int height);

	void init(int width, int height);
	void init();

	void load(NVGcontext* vg, String filename);
	void writeToPng(String filename);

	// Create a NanoVG image
	void createImage(NVGcontext* vg);
	void update8BitImageBuffer(NVGcontext* vg);
	void clear();

	void update(NVGcontext* vg);

	int width() const { return m_width; }
	int height() const { return m_height; }
	int channels() const { return m_channels; }
	int imageId() const { return m_imageId; }

	Color3 getPixel(int x, int y) const;
	void setPixel(int x, int y, const Color3& color);

	Pixel8_t getPixelData(int x, int y) const;
	void setPixel(int x, int y, Pixel8_t color);

	void requestUpdate() { m_needsUpdate = true; }

protected:
	String m_filename;

	int m_channels = 4; // needs to be 4 for rgba with nanovg create image func
	
	int m_width = 0;
	int m_height = 0;

	// float per channel image data, hopefully linear space. size = m_width * m_height
	Array<Color3> m_colorData;
	// 8-bit per channel image data, hopefully in sRGB space. size = m_width * m_height * m_channels
	Array<uint8_t> m_data;

	int m_imageId = -1; // NanoVG imageId
	bool m_needsUpdate = false; // When set to true, the image will be created (if needed) and updated to nanovg.
};

}
