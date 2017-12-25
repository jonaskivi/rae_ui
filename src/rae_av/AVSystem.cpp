#include <iostream>

#include "rae/visual/RenderSystem.hpp"

#include "fx/HdrFlow.hpp"
#include "fx/OpticalFlow.hpp"

#include "rae_av/AVSystem.hpp"

using namespace rae::av;

AVSystem::AVSystem(RenderSystem& renderSystem)
: m_renderSystem(renderSystem)
{
	av_register_all();
}

AssetId AVSystem::loadAsset(String filepath)
{
	m_mediaAssets.emplace_back(filepath);
	return m_mediaAssets.size() - 1;
}

bool AVSystem::hasAsset(AssetId id)
{
	return m_mediaAssets.size() > id;
}

MediaAsset& AVSystem::getAsset(AssetId id)
{
	return m_mediaAssets[id];
}

bool AVSystem::update(double time, double deltaTime, std::vector<Entity>&)
{
	return false; // for now
}

void AVSystem::copyFrameToImage(AVFrame* frameRGB, ImageBuffer& image)
{
	if (image.width != frameRGB->width or image.height != frameRGB->height)
		image.init(frameRGB->width, frameRGB->height);

	for (int y = 0; y < frameRGB->height; ++y)
	{
		for (int x = 0; x < frameRGB->width; ++x)
		{
			int p = x * 3 + y * frameRGB->linesize[0];
			uint8_t r = frameRGB->data[0][p];
			uint8_t g = frameRGB->data[0][p+1];
			uint8_t b = frameRGB->data[0][p+2];
			image.data[(y*image.width*image.channels) + (x*image.channels) + 0] = r;
			image.data[(y*image.width*image.channels) + (x*image.channels) + 1] = g;
			image.data[(y*image.width*image.channels) + (x*image.channels) + 2] = b;
		}
	}

	image.needsUpdate = true;
}