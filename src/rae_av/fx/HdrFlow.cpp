#ifdef USE_RAE_AV
#include "rae_av/fx/HdrFlow.hpp"

using namespace cv;
using namespace rae;
using namespace rae::av;

HdrFlow::HdrFlow(OpticalFlowMethod method)
{
	// Create two optical flows
	m_flows.emplace_back(method);
	m_flows.emplace_back(method);
}

void HdrFlow::pushFrame(AVFrame* frameRGB)
{
	if (m_flows.size() < 2)
		return;

	if (m_frameCount % 2 == 0)
	{
		m_flows[0].replaceFrame0(frameRGB);
	}
	else
	{
		m_flows[1].pushFrame(frameRGB);
	}

	if (m_flows[1].getState() == EffectNodeState::Processing)
		setState(EffectNodeState::Processing);

	/*bool allProcessing = true;
	for (auto&& flow : m_flows)
	{
		if (flow.getState() != EffectNodeState::Processing)
			allProcessing = false;
	}

	if (allProcessing)
		setState(EffectNodeState::Processing);
	*/

	/* RAE_TODO REMOVE:
	if (m_frameCount % 2 == 0)
	{
		
	}
	else
	{
		//setState(EffectNodeState::Processing);
	}
	*/
	m_frameCount++;
}

void HdrFlow::update(double time, double deltaTime)
{
	m_flows[1].update(time, deltaTime);
	if (m_flows[1].getState() == EffectNodeState::Done)
		setState(EffectNodeState::Done);

	/*bool allDone = true;
	for (auto&& flow : m_flows)
	{
		flow.update(time, deltaTime);
		if (flow.getState() != EffectNodeState::Done)
			allDone = false;
	}

	if (allDone)
	{
		setState(EffectNodeState::Done);
	}
	*/
}

void HdrFlow::writeFrameToImage(ImageBuffer& image)
{
	//const Mat& frameTwo0 = m_flows[0].getOutputAtTime(0.5f);
	//const Mat& frameTwo1 = m_flows[1].getOutputAtTime(0.0f);

	const Mat& frameTwo0 = m_flows[0].getOutputAtTime(0.0f);
	const Mat& frameTwo1 = m_flows[1].getOutputAtTime(0.5f);

	cv::addWeighted(frameTwo0, 0.5f, frameTwo1, 0.5f, 0.0f, m_output);

	// RAE_TODO MOVE METHOD TO SOME OTHER CLASS:
	m_flows[0].copyMatToImage(m_output, image);
}

void HdrFlow::writeFrameToDiskAndImage(String filepath, ImageBuffer& image)
{
	//TODO frameOne
	const Mat& frameTwo0 = m_flows[0].getOutputAtTime(0.0f);
	const Mat& frameTwo1 = m_flows[1].getOutputAtTime(0.5f);

	cv::addWeighted(frameTwo0, 1.0f - m_exposureWeight, frameTwo1, m_exposureWeight, 0.0f, m_output);

	// RAE_TODO MOVE METHOD TO SOME OTHER CLASS:
	m_flows[0].copyMatToImage(m_output, image);

	String outFolder = filepath;
		int numberOfZeroes = 6;
		String tempString = std::to_string(m_outFrameCount);
		String outFile = String(numberOfZeroes - tempString.length(), '0') + tempString;
		m_flows[0].writeMatToPng(outFolder + outFile + ".png", m_output);

	m_outFrameCount++;
}

void HdrFlow::waitForData()
{
	m_flows[0].waitForData();
	m_flows[1].waitForData();
	setState(EffectNodeState::WaitingForData);
}
#endif
