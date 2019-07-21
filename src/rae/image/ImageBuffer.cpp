#include "rae/image/ImageBuffer.hpp"

#include <array>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "loguru/loguru.hpp"
#include "rae/core/Utils.hpp"

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

template <typename T>
ImageBuffer<T>::ImageBuffer()
{
}

template <typename T>
ImageBuffer<T>::ImageBuffer(int width, int height) :
	m_width(width),
	m_height(height)
{
	init();
}

template <typename T>
void ImageBuffer<T>::init(int width, int height)
{
	m_width = width;
	m_height = height;

	init();
}

template <typename T>
void ImageBuffer<T>::init()
{
	if (m_width == 0 || m_height == 0)
	{
		assert(0);
		return;
	}

	if (m_data.size() > 0)
		m_data.clear();

	m_data.reserve(m_width * m_height * m_channels);
	for (int i = 0; i < m_width * m_height; ++i)
	{
		m_data.push_back(0);
		m_data.push_back(0);
		m_data.push_back(0);
		m_data.push_back(255);
	}
}

template <typename T>
void ImageBuffer<T>::clear()
{
	std::fill(m_data.begin(), m_data.end(), 0);
}

template <typename T>
Pixel4<T> ImageBuffer<T>::getPixel(int x, int y) const
{
	Pixel4<T> pixel;
	pixel[Channel::R] = m_data[(y*m_width*m_channels) + (x*m_channels) + 0];
	pixel[Channel::G] = m_data[(y*m_width*m_channels) + (x*m_channels) + 1];
	pixel[Channel::B] = m_data[(y*m_width*m_channels) + (x*m_channels) + 2];
	pixel[Channel::A] = m_data[(y*m_width*m_channels) + (x*m_channels) + 3];
	return pixel;
}

template <typename T>
void ImageBuffer<T>::setPixel(int x, int y, Pixel4<T> color)
{
	m_data[(y*m_width*m_channels) + (x*m_channels) + 0] = color[Channel::R];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 1] = color[Channel::G];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 2] = color[Channel::B];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 3] = color[Channel::A];
}

template <typename T>
Pixel3<T> ImageBuffer<T>::getPixel3(int x, int y) const
{
	Pixel3<T> pixel;
	pixel[Channel::R] = m_data[(y*m_width*m_channels) + (x*m_channels) + 0];
	pixel[Channel::G] = m_data[(y*m_width*m_channels) + (x*m_channels) + 1];
	pixel[Channel::B] = m_data[(y*m_width*m_channels) + (x*m_channels) + 2];
	return pixel;
}

template <typename T>
void ImageBuffer<T>::setPixel3(int x, int y, Pixel3<T> color)
{
	m_data[(y*m_width*m_channels) + (x*m_channels) + 0] = color[Channel::R];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 1] = color[Channel::G];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 2] = color[Channel::B];
}

template <typename T>
Color3 ImageBuffer<T>::getPixelColor3(int x, int y) const
{
	return Color3(
		m_data[(y*m_width*m_channels) + (x*m_channels) + 0],
		m_data[(y*m_width*m_channels) + (x*m_channels) + 1],
		m_data[(y*m_width*m_channels) + (x*m_channels) + 2]
	);
}

template <typename T>
void ImageBuffer<T>::setPixelColor3(int x, int y, const Color3& color)
{
	m_data[(y*m_width*m_channels) + (x*m_channels) + 0] = color[Channel::R];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 1] = color[Channel::G];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 2] = color[Channel::B];
}

// NanoVG specific, uint8_t

template <typename T>
void ImageBuffer<T>::load(NVGcontext* vg, String filename)
{
	//CAN*T
}

template <>
void ImageBuffer<uint8_t>::load(NVGcontext* vg, String filename)
{
	if (m_imageId == -1)
	{
		m_filename = filename;
		m_imageId = nvgCreateImage(vg, filename.c_str(), 0);
	}
}

template <typename T>
void ImageBuffer<T>::writeToPng(String filename)
{
	//CAN*T
}

template <>
void ImageBuffer<uint8_t>::writeToPng(String filename)
{
	stbi_write_png(filename.c_str(), m_width, m_height, 4, &m_data[0], int(m_width) * 4);
}

template <typename T>
void ImageBuffer<T>::createImage(NVGcontext* vg)
{
	//CAN*T
}

template <>
void ImageBuffer<uint8_t>::createImage(NVGcontext* vg)
{
	if (m_imageId == -1 && vg != nullptr)
	{
		//LOG_F(INFO, "Creating image %ix%i", m_width, m_height);
		m_imageId = nvgCreateImageRGBA(vg, m_width, m_height, /*imageFlags*/0, &m_data[0]);
	}
	else
	{
		LOG_F(ERROR, "Failed to create an image %ix%i", m_width, m_height);
		if (m_imageId != -1)
			LOG_F(ERROR, "imageId was not -1. It was %i", m_imageId);
		if (vg == nullptr)
			LOG_F(ERROR, "NanoVG context was null.");
		assert(m_imageId == -1);
		assert(vg != nullptr);
	}
}

template <typename T>
void ImageBuffer<T>::update(NVGcontext* vg)
{
	//CAN*T
}

template <>
void ImageBuffer<uint8_t>::update(NVGcontext* vg)
{
	if (!m_needsUpdate)
		return;

	if (m_imageId == -1)
		createImage(vg);

	m_needsUpdate = false;
	nvgUpdateImage(vg, m_imageId, &m_data[0]);
}

template <typename T>
void ImageBuffer<T>::updateToNanoVG(NVGcontext* vg)
{
	//CAN*T
}

template <>
void ImageBuffer<uint8_t>::updateToNanoVG(NVGcontext* vg)
{
	nvgUpdateImage(vg, m_imageId, &m_data[0]);
}

template class rae::ImageBuffer<uint8_t>;
template class rae::ImageBuffer<float>;

void rae::renderImageNano(NVGcontext* vg, int imageId, float x, float y, float w, float h)
{
	nvgSave(vg);

	NVGpaint imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, imageId, 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, imgPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void rae::copy8BitImageBuffer(
	const ImageBuffer<float>& colorImageSource,
	ImageBuffer<uint8_t>& uintImageTarget)
{
	assert(colorImageSource.width() == uintImageTarget.width());//, "Image sizes must match.");
	assert(colorImageSource.height() == uintImageTarget.height());//, "Image sizes must match.");

	// update 8 bit image buffer
	{
		parallel_for(0, uintImageTarget.height(), [&](int y)
		{
			for (int x = 0; x < uintImageTarget.width(); ++x)
			{
				const Color3& linear = colorImageSource.getPixelColor3(x, y);

				Color3 color = gammaCorrectionAnd255(linear);

				uintImageTarget.setPixel3(x, y,
						{ uint8_t(color.r), uint8_t(color.g), uint8_t(color.b) }
					);
			}
		});
	}
}

void rae::update8BitImageBuffer(
	const ImageBuffer<float>& colorImageSource,
	ImageBuffer<uint8_t>& uintImageTarget,
	NVGcontext* vg)
{
	assert(colorImageSource.width() == uintImageTarget.width());//, "Image sizes must match.");
	assert(colorImageSource.height() == uintImageTarget.height());//, "Image sizes must match.");

	copy8BitImageBuffer(colorImageSource, uintImageTarget);
	uintImageTarget.updateToNanoVG(vg);
}

//----------------------------------------------------------------------------------------------------------------------

void FrameBufferImage::generateFBO(NVGcontext* vg)
{
	m_framebufferObject = nvgluCreateFramebuffer(vg, m_width, m_height, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (m_framebufferObject == nullptr)
	{
		LOG_F(ERROR, "Could not create FBO.");
		//assert(0);
	}
}

void FrameBufferImage::beginRenderFBO()
{
	nvgluBindFramebuffer(m_framebufferObject);
}

void FrameBufferImage::endRenderFBO()
{
	nvgluBindFramebuffer(NULL);
}

GLuint FrameBufferImage::textureId() const
{
	if (m_framebufferObject == nullptr)
		return 0;
	return m_framebufferObject->texture;
}
