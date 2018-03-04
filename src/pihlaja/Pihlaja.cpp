#include "pihlaja/Pihlaja.hpp"

Pihlaja::Pihlaja(GLFWwindow* glfwWindow) :
	m_engine(glfwWindow),
	#ifdef USE_RAE_AV
	m_avSystem(m_engine.getRenderSystem()),
	#endif
	m_screenImage(m_engine.getRenderSystem().getBackgroundImage()),
	m_input(m_engine.getInput()),
	m_uiSystem(m_engine.getUISystem())
{
	#ifdef USE_RAE_AV
	m_engine.addSystem(m_avSystem);
	#endif
	m_engine.addSystem(*this);

	using std::placeholders::_1;
	m_input.connectKeyEventHandler(std::bind(&Pihlaja::onKeyEvent, this, _1));

	#ifdef USE_RAE_AV
	//m_videoAssetId = m_avSystem.loadAsset("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/MVI_9132.MOV");
	//m_videoAssetId = m_avSystem.loadAsset("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glass/MVI_8882.MOV");
	m_videoAssetId = m_avSystem.loadAsset("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/test5.mov");

	////////m_hdrFlow.setExposureWeight(0.75f);
	#endif

	initUI();
}

void Pihlaja::initUI()
{
	auto& ui = m_uiSystem;

	Id panel = ui.createPanel(
		virxels(-600.0f, 250.0f, 0.0f),
		virxels(250.0f, 325.0f, 0.1f));

	ui.addLayout(panel);

	Id playButtonId = ui.createToggleButton("Play",
		virxels(0.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		m_play);
	ui.addToLayout(panel, playButtonId);

	Id rewindButton = ui.createButton("Rewind",
		virxels(-100.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		std::bind(&Pihlaja::rewind, this));
	ui.addToLayout(panel, rewindButton);

	Id debugNeedsFrameUpdateButtonId = ui.createTextBox("NeedsFrameUpdate",
		virxels(-100.0f, 380.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f));
	ui.bindActive(debugNeedsFrameUpdateButtonId, m_needsFrameUpdate);

	// Raytracer

	Id rayTracerButtonId = ui.createToggleButton("Render",
		virxels(0.0f, 150.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		m_engine.getRayTracerSystem().isEnabled());
	ui.addToLayout(panel, rayTracerButtonId);

	Id qualityButton = ui.createButton("Quality",
		virxels(-100.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		[&]()
		{
			m_engine.getRayTracerSystem().toggleBufferQuality();
		});
	ui.addToLayout(panel, qualityButton);

	Id saveImageButton = ui.createButton("Save Image",
		virxels(-200.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		[&]()
		{
			m_engine.getRayTracerSystem().writeToPng("./rae_ray_render.png");
		});
	ui.addToLayout(panel, saveImageButton);

	/*Id positionTextBox = ui.createTextBox(
		virxels(-100.0f, 380.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f));
	ui.bindValue(positionTextBox, m_selectionSystem.positionProperty or something);
	*/
}

void Pihlaja::togglePlay()
{
	m_play = !m_play;
}

void Pihlaja::rewind()
{
	#ifdef USE_RAE_AV
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
	#endif
	setNeedsFrameUpdate(true);
	m_frameCount = 0;
}

void Pihlaja::setNeedsFrameUpdate(bool value)
{
	m_needsFrameUpdate = value;

	m_engine.askForFrameUpdate();
}

void Pihlaja::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			case KeySym::space:
				togglePlay();
				break;
			case KeySym::Home:
				rewind();
			break;
			case KeySym::Tab:
				m_uiSystem.toggleIsEnabled();
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
				setNeedsFrameUpdate(true);
				break;
			/*case KeySym::E:
				if (m_videoRenderingState == VideoRenderingState::Player)
					m_videoRenderingState = VideoRenderingState::RenderToDisk;
				else m_videoRenderingState = VideoRenderingState::Player;
				setNeedsFrameUpdate(true);
				break;
				*/
			default:
			break;
		}
	}
}

void Pihlaja::run()
{
	m_engine.run();
}

// OpticalFlow version
UpdateStatus Pihlaja::update()
{
#ifdef USE_RAE_AV
	if (not m_play and not m_needsFrameUpdate)
	{
		return UpdateStatus::NotChanged;
	}

	if (m_opticalFlow.getState() == EffectNodeState::Nothing ||
		m_opticalFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (not m_avSystem.hasAsset(m_videoAssetId))
		{
			std::cout << "No asset found in AVSystem.\n";
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (not asset.isLoaded())
		{
			std::cout << "Asset is not loaded.\n";
			return UpdateStatus::NotChanged;
		}

		AVFrame* frameRGB = asset.pullFrame();
		m_frameCount++;

		if (m_videoRenderingState == VideoRenderingState::Player)
		{
			m_avSystem.copyFrameToImage(frameRGB, m_screenImage);
			setNeedsFrameUpdate(false);
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToScreen or
				 m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_opticalFlow.pushFrame(frameRGB);
		}

		//if (m_frameCount % 2 == 0)
		//{
		//	m_opticalFlow.pushFrame(frameRGB);
		//}
	}
	else if (m_opticalFlow.getState() == EffectNodeState::Processing)
	{
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen or
			m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			//m_opticalFlow.update(m_screenImage);
			m_opticalFlow.update();
		}
	}
	else if (m_opticalFlow.getState() == EffectNodeState::Done)
	{
		m_opticalFlow.update();
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen)
		{
			m_opticalFlow.writeFrameToImage(m_screenImage);
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_opticalFlow.writeFrameToDiskAndImage("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glassrender/",
				m_screenImage);
		}
		///////////NOT: m_opticalFlow.waitForData();
		
		//m_opticalFlow.copyMatToImage(m_opticalFlow.getoutput, m_screenImage);
	}

	//if (not m_opticalFlow.isDone())
	//{
	//	m_opticalFlow.process();
	//}

	//m_opticalFlow.update(m_screenImage);
#endif
	return (m_needsFrameUpdate || m_play) ? UpdateStatus::Changed : UpdateStatus::NotChanged;
}

/* HdrFlow version:
UpdateStatus Pihlaja::update()
{
	if (not m_play and not m_needsFrameUpdate)
		return UpdateStatus::NotChanged;

	if (m_hdrFlow.getState() == EffectNodeState::Nothing ||
		m_hdrFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (not m_avSystem.hasAsset(m_videoAssetId))
		{
			std::cout << "No asset found in AVSystem.\n";
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (not asset.isLoaded())
		{
			std::cout << "Asset is not loaded.\n";
			return UpdateStatus::NotChanged;
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
			//m_opticalFlow.update(m_screenImage);
			m_hdrFlow.update();
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
			m_hdrFlow.writeFrameToDiskAndImage("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glassrender/",
				m_screenImage);
		}
		m_hdrFlow.waitForData();
	}

	//if (not m_opticalFlow.isDone())
	//{
	//	m_opticalFlow.process();
	//}

	//m_opticalFlow.update(m_screenImage);

	return (m_needsFrameUpdate || m_play) ? UpdateStatus::Changed : UpdateStatus::NotChanged;
}
*/

