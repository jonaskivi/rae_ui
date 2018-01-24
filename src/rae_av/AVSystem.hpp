#pragma once
#ifdef USE_RAE_AV

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/image/ImageBuffer.hpp"

#include "rae_av/media/MediaAsset.hpp"
#include "rae_av/fx/HdrFlow.hpp"
#include "rae_av/fx/OpticalFlow.hpp"

namespace rae
{
class RenderSystem;

namespace av
{

class HdrFlow;
class OpticalFlow;

class AVSystem : public rae::ISystem
{
public:
	AVSystem(RenderSystem& renderSystem);

	String name() override { return "AVSystem"; }

	AssetId loadAsset(String filepath);

	// This AssetId system is not good, because it doesn't allow deleting or unloading assets.
	// We must use a more similar system to what the components are using in other systems.
	bool hasAsset(AssetId id);
	MediaAsset& getAsset(AssetId id); // Could return Optional instead.

	UpdateStatus update() override;
	void destroyEntities(const Array<Id>& entities) override;
	void defragmentTables() override;

	void copyFrameToImage(AVFrame* frameRGB, ImageBuffer& image);

private:

	RenderSystem&			m_renderSystem;

	Array<MediaAsset>		m_mediaAssets;
};

}
}
#endif
