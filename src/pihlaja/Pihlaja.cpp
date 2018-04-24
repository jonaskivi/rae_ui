#include "pihlaja/Pihlaja.hpp"

Pihlaja::Pihlaja(GLFWwindow* glfwWindow, NVGcontext* nanoVG) :
	m_engine(glfwWindow, nanoVG),
	#ifdef USE_RAE_AV
	m_avSystem(m_engine.renderSystem()),
	#endif
	m_screenImage(1920, 1080),
	m_input(m_engine.input()),
	m_assetSystem(m_engine.assetSystem()),
	m_uiSystem(m_engine.uiSystem())
{
	LOG_F(INFO, "Adding systems.");

	m_engine.addSystem(m_engine.input());

	m_engine.addSystem(m_engine.assetSystem());
	m_engine.addSystem(m_engine.sceneSystem());

	m_engine.addSystem(m_engine.editorSystem());
	m_engine.addSystem(m_engine.uiSystem());
	m_engine.addSystem(m_engine.rayTracer());
	m_engine.addSystem(m_engine.renderSystem());

	m_engine.addRenderer3D(m_engine.renderSystem());
	m_engine.addRenderer3D(m_engine.editorSystem());
	m_engine.addRenderer3D(m_engine.debugSystem());

	m_engine.addRenderer2D(m_engine.uiSystem());
	m_engine.addRenderer2D(m_engine.debugSystem());

	m_engine.assetSystem().createTestAssets();

	m_engine.sceneSystem().activeScene().createTestWorld2(m_engine.assetSystem());

	Scene& alternativeScene = m_engine.sceneSystem().createScene("Alternative");
	alternativeScene.createTestWorld(m_engine.assetSystem());

	auto& scene = m_engine.sceneSystem().activeScene();
	m_engine.rayTracer().updateScene(scene);

	//

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

	// RAE_TODO: Convert from this strange centered virtual pixel coordinate system to
	// a top-left corner millimeter based coordinate system. That kind of system should
	// be device agnostic, and together with some auto layout system, can possibly solve some of
	// the issues in other coordinate systems.

	// So 2D would be top-left Y down, for easy UI layout (and texture coordinates, reading direction etc.).
	// And 3D (world-space) would be Z-up right handed (Y-left, X-forward) for easy level design (architecture etc.).

	m_screenImageAssetId = m_assetSystem.createImage(1920, 1080);

	Id videoBufferImageBox = ui.createImageBox(
		m_screenImageAssetId,
		virxels(400.0f, 300.0f, 0.0f),
		virxels(500.0f, 300.0f, 0.1f));

	int sceneIndex = 0;
	Id viewport = ui.createViewport(sceneIndex,
		virxels(-402.0f, -200.0f, 0.0f),
		virxels(800.0f, 500.0f, 0.1f));

	int sceneIndex2 = 1;
	Id viewport2 = ui.createViewport(sceneIndex2,
		virxels(402.0f, -200.0f, 0.0f),
		virxels(800.0f, 500.0f, 0.1f));

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

	Id renderModeButton = ui.createButton("Render Mode",
		virxels(-100.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		[&]()
		{
			auto renderMode = m_engine.renderSystem().toggleRenderMode();

			if (renderMode == RenderMode::Rasterize)
				LOG_F(INFO, "renderMode: Rasterize");
			else if (renderMode == RenderMode::RayTrace)
				LOG_F(INFO, "renderMode: RayTrace");
			else if (renderMode == RenderMode::MixedRayTraceRasterize)
				LOG_F(INFO, "renderMode: Mixed");
			else LOG_F(INFO, "renderMode: %i", (int)renderMode);


			if (renderMode == RenderMode::Rasterize)
				m_engine.rayTracer().setIsEnabled(false);
			else m_engine.rayTracer().setIsEnabled(true);
		});
	ui.addToLayout(panel, renderModeButton);

	/*
	Id renderButtonId = ui.createToggleButton("Render",
		virxels(0.0f, 150.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		m_engine.renderSystem().isEnabled());
	ui.addToLayout(panel, renderButtonId);

	// Raytracer
	Id rayTracerButtonId = ui.createToggleButton("Raytrace",
		virxels(0.0f, 150.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		m_engine.rayTracer().isEnabled());
	ui.addToLayout(panel, rayTracerButtonId);
	*/

	Id qualityButton = ui.createButton("Quality",
		virxels(-100.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		[&]()
		{
			m_engine.rayTracer().toggleBufferQuality();
		});
	ui.addToLayout(panel, qualityButton);

	Id saveImageButton = ui.createButton("Save Image",
		virxels(-200.0f, 350.0f, 0.0f),
		virxels(98.0f, 25.0f, 0.1f),
		[&]()
		{
			m_engine.rayTracer().writeToPng("./rae_ray_render.png");
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
		LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
		return;
	}

	auto& asset = m_avSystem.getAsset(m_videoAssetId);
	if (not asset.isLoaded())
	{
		LOG_F(ERROR, "Asset is not loaded. id: %i", m_videoAssetId);
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

void Pihlaja::reactToInput(const Input& input)
{
	if (!m_engine.sceneSystem().hasActiveScene())
		return;

	Scene& scene = m_engine.sceneSystem().activeScene();
	auto& entitySystem = scene.entitySystem();

	if (input.getKeyState(KeySym::I))
	{
		scene.createRandomCubeEntity(m_assetSystem);
		scene.createRandomBunnyEntity(m_assetSystem);
		scene.createRandomBunnyEntity(m_assetSystem);
	}

	/*
	if (input.getKeyState(KeySym::O))
	{
		LOG_F(INFO, "Destroy biggestId: %i", entitySystem.biggestId());
		m_engine.destroyEntity((Id)getRandomInt(20, entitySystem.biggestId()));
	}
	*/

	if (input.getKeyState(KeySym::P))
	{
		m_engine.defragmentTablesAsync(); //RAE_TODO SceneSystem!
	}

	// TODO use KeySym::Page_Up
	if (input.getKeyState(KeySym::K)) { m_engine.rayTracer().minusBounces(); }
	if (input.getKeyState(KeySym::L)) { m_engine.rayTracer().plusBounces(); }
}

void Pihlaja::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			case KeySym::Escape:	m_engine.quit(); break;
			//case KeySym::R:			m_renderSystem.clearImageRenderer(); break;
			case KeySym::G:			m_engine.renderSystem().toggleRenderMode(); break;
			case KeySym::space:		togglePlay(); break;
			case KeySym::Home:		rewind(); break;
			case KeySym::Tab:
					m_uiSystem.toggleIsEnabled();
					m_engine.debugSystem().toggleIsEnabled();
					break;
			case KeySym::_1:		m_engine.sceneSystem().activateScene(0); break;
			case KeySym::_2:		m_engine.sceneSystem().activateScene(1); break;
			case KeySym::_3:		m_evenFrames = true; break;
			case KeySym::_4:		m_evenFrames = false; break;
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
			//RAE_OLD case KeySym::Y: m_rayTracer.toggleBufferQuality(); break;
			//RAE_OLD case KeySym::U: m_rayTracer.toggleFastMode(); break;
			//RAE_OLD case KeySym::H: m_rayTracer.toggleVisualizeFocusDistance(); break;
			//RAE_OLD case KeySym::_1: m_rayTracer.showScene(1); break;
			//RAE_OLD case KeySym::_2: m_rayTracer.showScene(2); break;
			//RAE_OLD case KeySym::_3: m_rayTracer.showScene(3); break;
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
	reactToInput(m_input);

#ifdef USE_RAE_AV
	if (not m_play and not m_needsFrameUpdate)
	{
		return UpdateStatus::NotChanged;
	}

	auto& screenImage = m_assetSystem.getImage(m_screenImageAssetId);

	if (m_opticalFlow.getState() == EffectNodeState::Nothing ||
		m_opticalFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (not m_avSystem.hasAsset(m_videoAssetId))
		{
			LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (not asset.isLoaded())
		{
			LOG_F(ERROR, "Asset is not loaded. id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		AVFrame* frameRGB = asset.pullFrame();
		m_frameCount++;

		if (m_videoRenderingState == VideoRenderingState::Player)
		{
			m_avSystem.copyFrameToImage(frameRGB, screenImage);
			screenImage.requestUpdate();
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
			//m_opticalFlow.update(screenImage);
			m_opticalFlow.update();
		}
	}
	else if (m_opticalFlow.getState() == EffectNodeState::Done)
	{
		m_opticalFlow.update();
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen)
		{
			m_opticalFlow.writeFrameToImage(screenImage);
			screenImage.requestUpdate();
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_opticalFlow.writeFrameToDiskAndImage("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glassrender/",
				screenImage);
			screenImage.requestUpdate();
		}
		///////////NOT: m_opticalFlow.waitForData();
		
		//m_opticalFlow.copyMatToImage(m_opticalFlow.getoutput, screenImage);
	}

	//if (not m_opticalFlow.isDone())
	//{
	//	m_opticalFlow.process();
	//}

	//m_opticalFlow.update(screenImage);
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
			LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (not asset.isLoaded())
		{
			LOG_F(ERROR, "Asset is not loaded. id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		AVFrame* frameRGB = asset.pullFrame();
		m_frameCount++;

		if (m_videoRenderingState == VideoRenderingState::Player)
		{
			if (m_needsFrameUpdate or (m_evenFrames and m_frameCount % 2 == 0))
				m_avSystem.copyFrameToImage(frameRGB, screenImage);
			else if (m_needsFrameUpdate or (not m_evenFrames and m_frameCount % 2))
				m_avSystem.copyFrameToImage(frameRGB, screenImage);
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
			//m_opticalFlow.update(screenImage);
			m_hdrFlow.update();
		}
	}
	else if (m_hdrFlow.getState() == EffectNodeState::Done)
	{
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen)
		{
			m_hdrFlow.writeFrameToImage(screenImage);
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToDisk)
		{
			m_hdrFlow.writeFrameToDiskAndImage("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glassrender/",
				screenImage);
		}
		m_hdrFlow.waitForData();
	}

	//if (not m_opticalFlow.isDone())
	//{
	//	m_opticalFlow.process();
	//}

	//m_opticalFlow.update(screenImage);

	return (m_needsFrameUpdate || m_play) ? UpdateStatus::Changed : UpdateStatus::NotChanged;
}
*/

