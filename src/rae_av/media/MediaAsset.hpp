#pragma once

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include "rae/asset/IAsset.hpp"

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
	AVCodecContext*			m_codecContextOriginal	= nullptr;
	AVCodecContext*			m_codecContext			= nullptr;
	AVFrame*				m_frame					= nullptr;
	AVFrame*				m_frameRGB				= nullptr;
	uint8_t*				m_frameBuffer			= nullptr; // buffer for m_frameRGB

	// Probably GPL so remove before distributing:
	struct SwsContext* m_swsContext;
};

}
}
