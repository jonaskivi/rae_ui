#pragma once

#ifdef USE_RAE_AV

#define __STDC_CONSTANT_MACROS // Some strange C thing...
extern "C"
{
	#include <libavutil/imgutils.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}
#endif

#include "rae/asset/IAsset.hpp"

#ifdef USE_RAE_AV
namespace rae
{
namespace av
{

using AssetId = int;

class MediaAsset : IAsset
{
public:
	MediaAsset();
	MediaAsset(String setFilepath);
	~MediaAsset();

	bool load(); // Load the mediafile. Return true on success.

	void unload(); // Free memory and resources.

	bool isLoaded() { return m_loaded; }

	AVFrame* pullFrame();
	void seekToStart();

protected:

	bool m_loaded = false;

	AVFormatContext*		m_formatContext			= nullptr;
	int						m_videoStreamIndex		= -1;
	AVCodecParameters*		m_codecParameters		= nullptr;
	AVCodecContext*			m_codecContext			= nullptr;
	AVFrame*				m_frame					= nullptr;
	AVFrame*				m_frameRGB				= nullptr;
	uint8_t*				m_frameBuffer			= nullptr; // buffer for m_frameRGB

	// Probably GPL so remove before distributing:
	struct SwsContext* m_swsContext;
};

}
}
#endif
