#include <iostream>
#include <ciso646>
#include <array>

#include "rae/core/Log.hpp"
#include "rae/core/Utils.hpp"
#include "rae/image/ImageBuffer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

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
{
}

ImageBuffer::ImageBuffer(int width, int height) :
	m_width(width),
	m_height(height)
{
	init();
}

void ImageBuffer::init(int width, int height)
{
	m_width = width;
	m_height = height;

	init();
}

void ImageBuffer::init()
{
	if (m_width == 0 || m_height == 0)
	{
		assert(0);
		return;
	}

	if (m_colorData.size() > 0)
		m_colorData.clear();
	if (m_data.size() > 0)
		m_data.clear();

	m_colorData.reserve(m_width * m_height);
	for (int i = 0; i < m_width * m_height; ++i)
	{
		m_colorData.push_back(vec3(0.5f, 0.5f, 0.5f));
	}
	m_data.reserve(m_width * m_height * m_channels);
	for (int i = 0; i < m_width * m_height; ++i)
	{
		m_data.push_back(0);
		m_data.push_back(0);
		m_data.push_back(0);
		m_data.push_back(255);
	}
}

void ImageBuffer::load(NVGcontext* vg, String filename)
{
	if (m_imageId == -1)
	{
		m_filename = filename;
		m_imageId = nvgCreateImage(vg, filename.c_str(), 0);
	}
}

void ImageBuffer::writeToPng(String filename)
{
	stbi_write_png(filename.c_str(), m_width, m_height, 4, &m_data[0], int(m_width) * 4);
}

void ImageBuffer::createImage(NVGcontext* vg)
{
	if (m_imageId == -1 && vg != nullptr)
	{
		//std::cout << "Creating image " << m_width << "x" << m_height << "\n";
		m_imageId = nvgCreateImageRGBA(vg, m_width, m_height, /*imageFlags*/0, &m_data[0]);
	}
	else
	{
		rae_log("Failed to create an image ", m_width, "x", m_height);
		if (m_imageId != -1)
			rae_log("imageId was not -1. It was ", m_imageId);
		if (vg == nullptr)
			rae_log("NanoVG context was null.");
		assert(m_imageId == -1);
		assert(vg != nullptr);
	}
}

void ImageBuffer::clear()
{
	std::fill(m_colorData.begin(), m_colorData.end(), vec3(0,0,0));
	//std::fill(m_data.begin(), m_data.end(), 0);
}

void ImageBuffer::update(NVGcontext* vg)
{
	if (not m_needsUpdate)
		return;

	if (m_imageId == -1)
		createImage(vg);

	m_needsUpdate = false;
	nvgUpdateImage(vg, m_imageId, &m_data[0]);
}

void ImageBuffer::update8BitImageBuffer(NVGcontext* vg)
{
	// update 8 bit image buffer
	{
		parallel_for(0, m_height, [&](int j)
		{
			for (int i = 0; i < m_width; ++i)
			{
				const vec3& linear = m_colorData[(j*m_width)+i];

				vec3 color = gammaCorrectionAnd255(linear);

				m_data[(j*m_width*m_channels) + (i*m_channels) + 0] = uint8_t(color.r);
				m_data[(j*m_width*m_channels) + (i*m_channels) + 1] = uint8_t(color.g);
				m_data[(j*m_width*m_channels) + (i*m_channels) + 2] = uint8_t(color.b);
			}
		});

		nvgUpdateImage(vg, m_imageId, &m_data[0]);
	}
}

vec3 ImageBuffer::getPixel(int x, int y) const
{
	return m_colorData[(y * m_width) + x];
}

void ImageBuffer::setPixel(int x, int y, const vec3& color)
{
	m_colorData[(y * m_width) + x] = color;
}

std::array<uint8_t, 4> ImageBuffer::getPixelData(int x, int y) const
{
	std::array<uint8_t, 4> pixel;
	pixel[Channel::R] = m_data[(y*m_width*m_channels) + (x*m_channels) + 0];
	pixel[Channel::G] = m_data[(y*m_width*m_channels) + (x*m_channels) + 1];
	pixel[Channel::B] = m_data[(y*m_width*m_channels) + (x*m_channels) + 2];
	return pixel;
}

void ImageBuffer::setPixel(int x, int y, std::array<uint8_t, 4> color)
{
	m_data[(y*m_width*m_channels) + (x*m_channels) + 0] = color[Channel::R];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 1] = color[Channel::G];
	m_data[(y*m_width*m_channels) + (x*m_channels) + 2] = color[Channel::B];
}
