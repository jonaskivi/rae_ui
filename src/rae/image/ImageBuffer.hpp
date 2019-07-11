#pragma once

#include <GL/glew.h>

#include <array>

#include "rae/core/Types.hpp"

struct NVGcontext;
struct NVGLUframebuffer;

namespace rae
{

enum Channel
{
	R,
	G,
	B,
	A
};

template<class T>
using Pixel4 = std::array<T, 4>;

template<class T>
using Pixel3 = std::array<T, 3>;

using float4 = std::array<float, 4>;
using float3 = std::array<float, 3>;;

using uint4 = std::array<uint8_t, 4>;;
using uint3 = std::array<uint8_t, 3>;;

template <typename T>
class ImageBuffer
{
public:
	ImageBuffer();
	ImageBuffer(int width, int height);

	void init(int width, int height);
	void init();

	void clear();

	int width() const { return m_width; }
	int height() const { return m_height; }
	int channels() const { return m_channels; }

	// RGBA pixels
	Pixel4<T> getPixel(int x, int y) const;
	void setPixel(int x, int y, Pixel4<T> pixel);

	// RGB pixels
	Pixel3<T> getPixel3(int x, int y) const;
	void setPixel3(int x, int y, Pixel3<T> pixel);

	// RGB pixels
	Color3 getPixelColor3(int x, int y) const;
	void setPixelColor3(int x, int y, const Color3& color);

// Only for uint8_t:
	void load(NVGcontext* vg, String filename);
	void writeToPng(String filename);

	// Create a NanoVG image
	void createImage(NVGcontext* vg);
	// NanoVG image ID
	int imageId() const { return m_imageId; }
	void update(NVGcontext* vg);
	void requestUpdate() { m_needsUpdate = true; }
	void updateToNanoVG(NVGcontext* vg);

protected:
	int m_channels = 4; // needs to be 4 for rgba with nanovg create image func

	int m_width = 0;
	int m_height = 0;

	// Array of image data. size = m_width * m_height * m_channels
	Array<T> m_data;

// Only for uint8_t:
	String m_filename;

	int m_imageId = -1; // NanoVG imageId
	bool m_needsUpdate = false; // When set to true, the image will be created (if needed) and updated to nanovg.
};

void renderImageNano(NVGcontext* vg, int imageId, float x, float y, float w, float h);

void copy8BitImageBuffer(
	const ImageBuffer<float>& colorImageSource,
	ImageBuffer<uint8_t>& uintImageTarget);

void update8BitImageBuffer(
	const ImageBuffer<float>& colorImageSource,
	ImageBuffer<uint8_t>& uintImageTarget,
	NVGcontext* vg);

class FrameBufferImage
{
public:
	FrameBufferImage(int width = 512, int height = 512) :
		m_width(width),
		m_height(height)
	{
	}

	int width() const { return m_width; }
	int height() const { return m_height; }

	void generateFBO(NVGcontext* vg);
	bool isValid() { return m_framebufferObject != nullptr; }

	void beginRenderFBO();
	void endRenderFBO();

	GLuint textureId() const;

protected:
	int m_width = 0;
	int m_height = 0;

	NVGLUframebuffer* m_framebufferObject = nullptr;
};

}
