#pragma once
#ifdef USE_RAE_AV
#include "rae_av/fx/OpticalFlow.hpp"

namespace rae
{
namespace av
{

class HdrFlow
{
public:
	HdrFlow(OpticalFlowMethod method = OpticalFlowMethod::DeepFlow);

	EffectNodeState getState() { return m_state; }

	void pushFrame(AVFrame* frameRGB);

	void update();
	void writeFrameToImage(ImageBuffer<uint8_t>& image);
	void writeFrameToDiskAndImage(String filepath, ImageBuffer<uint8_t>& image);
	void waitForData();

	void setExposureWeight(float value) { m_exposureWeight = value;}

private:

	void setState(EffectNodeState state)
	{
		LOG_F(INFO, "HdrFlow State: %i", (int)state);
		m_state = state;
	}

	EffectNodeState m_state = EffectNodeState::Nothing;

	int m_frameCount = 0;
	int m_outFrameCount = 0;

	Array<OpticalFlow> m_flows;
	cv::Mat m_output;

	// Settings
	float m_exposureWeight = 0.5;
};

}
}
#endif
