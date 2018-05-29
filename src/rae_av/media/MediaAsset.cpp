#ifdef USE_RAE_AV
#include "loguru/loguru.hpp"

#include "rae_av/media/MediaAsset.hpp"

using namespace rae;
using namespace rae::av;

MediaAsset::MediaAsset()
{
}

MediaAsset::MediaAsset(String setFilepath)
: IAsset(setFilepath)
{
	m_loaded = load();
}

MediaAsset::~MediaAsset()
{
	unload();
}

bool MediaAsset::load()
{
	m_formatContext = nullptr;
	// Open video file
	if (avformat_open_input(&m_formatContext, m_filepath.c_str(), nullptr, nullptr) != 0)
	{
		LOG_F(ERROR, "Couldn't open file: %s", m_filepath.c_str());
		return false;
	}
	else
	{
		LOG_F(INFO, "FFMPEG opened file nicely!: %s", m_filepath.c_str());
	}

	// Retrieve stream information
	if (avformat_find_stream_info(m_formatContext, nullptr) < 0)
	{
		LOG_F(ERROR, "Couldn't find stream information: %s", m_filepath.c_str());
		return false;
	}

	LOG_F(INFO, "Video file info: %s"
		" Duration: %f"
		" Stream count: %i"
		" Bit rate: %i",
		m_filepath.c_str(),
		(m_formatContext->duration / AV_TIME_BASE),
		m_formatContext->nb_streams,
		m_formatContext->bit_rate);

	// Dump information about file onto standard error (doesn't seem to work with XCode...)
	//av_dump_format(m_formatContext, 0, m_filepath.c_str(), 0);

	// Find the first video stream
	m_videoStreamIndex = -1;
	for (int i = 0; i < m_formatContext->nb_streams; ++i)
	{
		if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoStreamIndex = i;
			break;
		}
	}

	if (m_videoStreamIndex == -1)
	{
		LOG_F(ERROR, "Didn't find a video stream: ", m_filepath.c_str());
		return false;
	}

	// Get a pointer to the codec context for the video stream
	m_codecParameters = m_formatContext->streams[m_videoStreamIndex]->codecpar;

	// Find the decoder for the video stream
	AVCodec* pCodec = avcodec_find_decoder(m_codecParameters->codec_id);
	if (pCodec == nullptr)
	{
		LOG_F(ERROR, "Unsupported codec!");
		return false;
	}
	// Copy context
	m_codecContext = nullptr;
	m_codecContext = avcodec_alloc_context3(pCodec);

	avcodec_parameters_to_context(m_codecContext, m_codecParameters);

	// Open codec
	if (avcodec_open2(m_codecContext, pCodec, /*options*/nullptr) < 0)
	{
		LOG_F(ERROR, "Could not open codec.");
		return false;
	}

	LOG_F(INFO, "Video file width: %i height: %i", m_codecContext->width, m_codecContext->height);

	// Allocate video frames
	m_frame = av_frame_alloc();
	m_frameRGB = av_frame_alloc();
	if (m_frame == nullptr || m_frameRGB == nullptr)
	{
		LOG_F(ERROR, "Could not allocate video frames.");
		return false;
	}

	// Determine required m_frameBuffer size and allocate m_frameBuffer
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_codecContext->width, m_codecContext->height, 32);
	m_frameBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

	// Assign appropriate parts of m_frameBuffer to image planes in m_frameRGB
	av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize, m_frameBuffer, AV_PIX_FMT_RGB24,
		m_codecContext->width, m_codecContext->height, 32);

	// initialize SWS context for software scaling
	m_swsContext = sws_getContext(m_codecContext->width,
		m_codecContext->height,
		m_codecContext->pix_fmt,
		m_codecContext->width,
		m_codecContext->height,
		AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		nullptr,
		nullptr,
		nullptr);

	return true;
}

AVFrame* MediaAsset::pullFrame()
{
	int frameFinished = 0;
	AVPacket packet;

	int frameCount = 0;

	int ret = 0;
	while (frameFinished == 0 && av_read_frame(m_formatContext, &packet) >= 0)
	{
		// Is this a packet from the video stream?
		if (packet.stream_index == m_videoStreamIndex)
		{
			do
			{
				ret = avcodec_send_packet(m_codecContext, &packet);
			} while(ret == AVERROR(EAGAIN));

			if (ret == AVERROR_EOF || ret == AVERROR(EINVAL))
			{
				printf("AVERROR(EAGAIN): %d, AVERROR_EOF: %d, AVERROR(EINVAL): %d\n", AVERROR(EAGAIN), AVERROR_EOF, AVERROR(EINVAL));
				printf("fe_read_frame: Frame getting error (%d)!\n", ret);
				continue;
				//return 0;
			}

			ret = avcodec_receive_frame(m_codecContext, m_frame);

			// RAE_TODO check for errors in ret again.

			frameFinished = true;

			// Did we get a video frame?
			if (frameFinished)
			{
				frameCount++;

				// Convert the image from its native format to RGB
				sws_scale(m_swsContext, (uint8_t const * const *)m_frame->data,
					m_frame->linesize, 0, m_codecContext->height,
					m_frameRGB->data, m_frameRGB->linesize);

				m_frameRGB->width = m_codecContext->width;
				m_frameRGB->height = m_codecContext->height;

				// Save the frame to disk
				/* RAE_TODO REMOVE:
				if (frameCount <= 2)
				{
					// RAE_TODO: SaveFrame(m_frameRGB, m_codecContext->width, m_codecContext->height, frameCount);
					if (frameCount % 2 != 0)
					{
						LOG_F(INFO, "pushFrame1.);
						m_opticalFlow.pushFrame1(m_frameRGB);
					}
					else
					{
						LOG_F(INFO, "pushFrame2.");
						m_opticalFlow.pushFrame2(m_frameRGB);
					}
				}
				*/
			}
		}

		av_packet_unref(&packet);
	}

	return m_frameRGB;
}

void MediaAsset::seekToStart()
{
	int64_t timestamp = 0;
	av_seek_frame(m_formatContext, m_videoStreamIndex, timestamp, /*flags:*/0);
}

void MediaAsset::unload()
{
	// Free the RGB image
	av_free(m_frameBuffer);
	av_free(m_frameRGB);

	// Free the YUV frame
	av_free(m_frame);

	// Close the codecs
	avcodec_close(m_codecContext);
	// Probably not needed to close codec parameters?: avcodec_close(m_codecParameters);

	// Close the video file
	avformat_close_input(&m_formatContext);
}
#endif
