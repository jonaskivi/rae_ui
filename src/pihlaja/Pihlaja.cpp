#include "pihlaja/Pihlaja.hpp"

Pihlaja::Pihlaja(GLFWwindow* glfwWindow)
	: m_engine(glfwWindow),
	m_avSystem(m_engine.getRenderSystem()),
	m_screenImage(m_engine.getRenderSystem().getBackgroundImage()),
	m_input(m_engine.getInput())
{
	m_engine.addSystem(m_avSystem);
	m_engine.addSystem(*this);

	using std::placeholders::_1;
	m_input.connectKeyEventHandler(std::bind(&Pihlaja::onKeyEvent, this, _1));

	m_videoAssetId = m_avSystem.loadAsset("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/MVI_9001.MOV");
	////////m_hdrFlow.setExposureWeight(0.75f);
}

void Pihlaja::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KEY_PRESS)
	{
		switch (input.key.value)
		{
			case KeySym::space:
				m_play = not m_play;
				break;
			case KeySym::Home:
			{
				if (not m_avSystem.hasAsset(m_videoAssetId))
				{
					std::cout << "No asset found in AVSystem.\n";
					return;
				}

				auto& asset = m_avSystem.getAsset(m_videoAssetId);
				if (not asset.isLoaded())
				{
					std::cout << "Asset is not loaded.\n";
					return;
				}

				asset.seekToStart();
				m_needsFrameUpdate = true;
				m_frameCount = 0;
			}
			break;
			case KeySym::_1:
				m_evenFrames = true;
				break;
			case KeySym::_2:
				m_evenFrames = false;
				break;
			case KeySym::R:
				if (m_videoRenderingState == VideoRenderingState::Player)
					m_videoRenderingState = VideoRenderingState::RenderToScreen;
				else m_videoRenderingState = VideoRenderingState::Player;
				m_needsFrameUpdate = true;
				break;
			case KeySym::E:
				if (m_videoRenderingState == VideoRenderingState::Player)
					m_videoRenderingState = VideoRenderingState::RenderToDisk;
				else m_videoRenderingState = VideoRenderingState::Player;
				m_needsFrameUpdate = true;
				break;
			default:
			break;
		}
	}
}

void Pihlaja::run()
{
	m_engine.run();
}

bool Pihlaja::update(double time, double deltaTime, std::vector<Entity>&)
{
	if (not m_play and not m_needsFrameUpdate)
		return false;

	if (m_hdrFlow.getState() == EffectNodeState::Nothing ||
		m_hdrFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (not m_avSystem.hasAsset(m_videoAssetId))
		{
			std::cout << "No asset found in AVSystem.\n";
			return false;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (not asset.isLoaded())
		{
			std::cout << "Asset is not loaded.\n";
			return false;
		}

		AVFrame* frameRGB = asset.pullFrame();
		m_frameCount++;

		if (m_videoRenderingState == VideoRenderingState::Player)
		{
			if (m_needsFrameUpdate or (m_evenFrames and m_frameCount % 2 == 0))
				m_avSystem.copyFrameToImage(frameRGB, m_screenImage);
			else if (m_needsFrameUpdate or (not m_evenFrames and m_frameCount % 2))
				m_avSystem.copyFrameToImage(frameRGB, m_screenImage);
			m_needsFrameUpdate = false;
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToScreen or
				 m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_hdrFlow.pushFrame(frameRGB);
		}

		//if (m_frameCount % 2 == 0)
		//{
		//	m_opticalFlow.pushFrame(frameRGB);
		//}
	}
	else if (m_hdrFlow.getState() == EffectNodeState::Processing)
	{
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen or
			m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			//m_opticalFlow.update(time, deltaTime, m_screenImage);
			m_hdrFlow.update(time, deltaTime);
		}
	}
	else if (m_hdrFlow.getState() == EffectNodeState::Done)
	{
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen)
		{
			m_hdrFlow.writeFrameToImage(m_screenImage);
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_hdrFlow.writeFrameToDiskAndImage("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/testrender/",
				m_screenImage);
		}
		m_hdrFlow.waitForData();
	}

	/*
	if (not m_opticalFlow.isDone())
	{
		m_opticalFlow.process();
	}

	m_opticalFlow.update(time, deltaTime, m_screenImage);
	*/
	return m_needsFrameUpdate || m_play;
}
