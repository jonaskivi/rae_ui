#ifdef USE_RAE_AV
#include <array>

#include "rae/core/Utils.hpp"
#include "rae/visual/RenderSystem.hpp"

#include "fx/HdrFlow.hpp"
#include "fx/OpticalFlow.hpp"

#include "rae_av/AVSystem.hpp"

using namespace rae::av;

AVSystem::AVSystem(RenderSystem& renderSystem) :
	ISystem("AVSystem"),
	m_renderSystem(renderSystem)
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

rae::UpdateStatus AVSystem::update()
{
	return rae::UpdateStatus::NotChanged; // for now
}

void AVSystem::destroyEntities(const Array<Id>& entities)
{
}

void AVSystem::defragmentTables()
{
}

void AVSystem::copyFrameToImage(AVFrame* frameRGB, ImageBuffer<uint8_t>& image)
{
	if (image.width() != frameRGB->width or image.height() != frameRGB->height)
	{
		image.init(frameRGB->width, frameRGB->height);
	}

	parallel_for(0, frameRGB->height, [&](int y)
	{
		for (int x = 0; x < frameRGB->width; ++x)
		{
			int p = x * 3 + y * frameRGB->linesize[0];
			uint8_t r = frameRGB->data[0][p];
			uint8_t g = frameRGB->data[0][p+1];
			uint8_t b = frameRGB->data[0][p+2];
			image.setPixel(x, y, {r, g, b, 255});
		}
	});

	image.requestUpdate();
}
#endif
