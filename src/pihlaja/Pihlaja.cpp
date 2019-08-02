#include "pihlaja/Pihlaja.hpp"

Pihlaja::Pihlaja() :
	ISystem("PihlajaSystem"),
	m_engine("Pihlaja"),
	#ifdef USE_RAE_AV
	m_avSystem(m_engine.renderSystem()),
	#endif
	m_screenImage(1920, 1080),
	m_input(m_engine.input()),
	m_assetSystem(m_engine.assetSystem()),
	m_uiSystem(m_engine.uiSystem())
{
	LOG_F(INFO, "Adding systems.");

	m_engine.addBaseSystems();

	m_engine.addSystem(m_engine.assetSystem());
	m_engine.addSystem(m_engine.sceneSystem());

	m_engine.addSystem(m_engine.uiSystem());
	m_engine.addSystem(m_engine.rayTracer());
	m_engine.addSystem(m_engine.renderSystem());

	m_engine.addSystem(m_engine.debugSystem());

	m_engine.addRenderer3D(m_engine.renderSystem());
	m_engine.addRenderer3D(m_engine.rayTracer()); // This is just the debug rendering.
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

	#ifdef __linux__
		m_videoAssetId = m_avSystem.loadAsset("/home/joonaz/Videot/test4.mov");
	#elif __APPLE__
		m_videoAssetId = m_avSystem.loadAsset("/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/test4.mov");
	#endif

	////////m_hdrFlow.setExposureWeight(0.75f);
	#endif

	initUI();
}

void Pihlaja::initUI()
{
	auto& uiSystem = m_uiSystem;
	auto& windowSystem = m_engine.windowSystem();

	UIScene& ui = uiSystem.defaultScene();
	uiSystem.connectWindowToScene(windowSystem.modifyMainWindow(), ui);

	auto& trans = ui.transformSystem();

	// RAE_TODO: Convert from this strange centered virtual pixel coordinate system to
	// a top-left corner millimeter based coordinate system. That kind of system should
	// be device agnostic, and together with some auto layout system, can possibly solve some of
	// the issues in other coordinate systems.

	// So 2D would be top-left Y down, for easy UI layout (and texture coordinates, reading direction etc.).
	// And 3D (world-space) would be Z-up right handed (Y-left, X-forward) for easy level design (architecture etc.).

	// Viewports for 3D and for raytracing. These should be defined first, because currently we can only draw UI on
	// top of the 3D viewports. Not sure if it is worth it to try and render viewports to textures just because
	// we'd want to draw UI under them. No, it wouldn't be worth it.

	Id rootId = ui.rootId();
	ui.addGridLayout(rootId, 2, 2);

	int sceneIndex = 0;
	Id viewport = ui.createAdvancedViewport(sceneIndex,
		vec3(130.0f, 80.0f, 0.0f),
		vec3(250.0f, 150.0f, 1.0f));
	trans.addChild(rootId, viewport);

	int sceneIndex2 = 1;
	Id viewport2 = ui.createAdvancedViewport(sceneIndex2,
		vec3(130.0f, 235.0f, 0.0f),
		vec3(250.0f, 150.0f, 1.0f));
	trans.addChild(rootId, viewport2);

	// Video things

	m_screenImageAssetId = m_assetSystem.createImage(1920, 1080);

	#ifndef _WIN32
	Id videoBufferImageBox = ui.createImageBox(
		m_screenImageAssetId,
		vec3(385.0f, 80.0f, 0.0f),
		vec3(250.0f, 150.0f, 1.0f));
	ui.addDraggable(videoBufferImageBox);
	ui.addMaximizerAndButton(videoBufferImageBox);
	trans.addChild(rootId, videoBufferImageBox);
	#endif

	Id videoControls = ui.createPanel(
		vec3(60.0f, 110.0f, 0.0f),
		vec3(100.0f, 50.0f, 1.0f));
	ui.addDraggable(videoControls);
	ui.addStackLayout(videoControls);

	float TitleFontSize = 22.0f;

	Id videoControlsText = ui.createTextBox("Video Controls",
		vec3(0.0f, 38.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		TitleFontSize);
		trans.addChild(videoControls, videoControlsText);

	Id playButton = ui.createToggleButton("Play",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		m_play);
	trans.addChild(videoControls, playButton);

	Id rewindButton = ui.createButton("Rewind",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		std::bind(&Pihlaja::rewind, this));
	trans.addChild(videoControls, rewindButton);

	#ifdef _WIN32
		Id noVideoText = ui.createTextBox("No video playback on Windows for now.",
		vec3(0.0f, 38.0f, 0.0f),
		vec3(100.0f, 10.0f, 1.0f));
		trans.addChild(videoControls, noVideoText);
	#endif

	// Raytracer Controls

	Id panel = ui.createPanel(
		vec3(200.0f, 130.0f, 0.0f),
		vec3(120.0f, 100.0f, 1.0f));
	ui.addDraggable(panel);
	ui.addMaximizer(panel);
	ui.addStackLayout(panel);

	Id panelTitleText = ui.createTextBox("Raytracer Controls",
		vec3(0.0f, 38.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		TitleFontSize);
	trans.addChild(panel, panelTitleText);

	Id panelMaximizeButton = ui.createButton("Maximize",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		[&, panel]()
		{
			ui.toggleMaximizer(panel);
		});
	trans.addChild(panel, panelMaximizeButton);

	Id renderModeButton = ui.createButton("Render Mode",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
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
	trans.addChild(panel, renderModeButton);

	Id qualityButton = ui.createButton("Toggle Quality",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		[&]()
		{
			m_engine.rayTracer().toggleBufferQuality();
		});
	trans.addChild(panel, qualityButton);

	Id saveImageButton = ui.createButton("Save Image",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f),
		[&]()
		{
			m_engine.rayTracer().writeToPng("./rae_ray_render.png");
		});
	trans.addChild(panel, saveImageButton);

	bool isMultilineText = true;
	Id propertiesText = ui.createTextBox("Nothing selected or hovered.",
		vec3(0.0f, 0.0f, 0.0f),
		vec3(120.0f, 120.0f, 1.0f),
		14.0f,
		isMultilineText);
	ui.connectUpdater(propertiesText,
		[&](Id id)
		{
			auto& text = ui.modifyText(id);
			const auto& sceneSystem = m_engine.sceneSystem();
			String setText;
			if (sceneSystem.hasActiveScene())
			{
				const auto& scene = sceneSystem.activeScene();
				const auto& selection = scene.selectionSystem();
				const auto& trans = scene.transformSystem();

				setText += "Active scene: " + scene.name() + "\n";

				if (selection.isSelection())
				{
					setText += trans.toString(selection.anySelected());
				}
				else if (selection.isAnyHovered())
				{
					setText += trans.toString(selection.hovered());
				}
				else
				{
					setText += "Nothing selected or hovered.";
				}
			}
			else
			{
				setText += "No active scene.";
			}
			text.text = setText;
		});

	trans.addChild(panel, propertiesText);

	/*Id positionTextBox = ui.createTextBox(
		vec3(-100.0f, 380.0f, 0.0f),
		vec3(98.0f, 25.0f, 1.0f));
	ui.bindValue(positionTextBox, m_selectionSystem.positionProperty or something);
	*/

	/*
	Id debugNeedsFrameUpdateButton = ui.createTextBox("NeedsFrameUpdate",
		vec3(0.0f, 38.0f, 0.0f),
		vec3(50.0f, 10.0f, 1.0f));
	ui.bindActive(debugNeedsFrameUpdateButtonId, m_needsFrameUpdate);
	trans.addChild(panel, debugNeedsFrameUpdateButton);
	*/
}

void Pihlaja::togglePlay()
{
	m_play = !m_play;
}

void Pihlaja::rewind()
{
	#ifdef USE_RAE_AV
	if (!m_avSystem.hasAsset(m_videoAssetId))
	{
		LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
		return;
	}

	auto& asset = m_avSystem.getAsset(m_videoAssetId);
	if (!asset.isLoaded())
	{
		LOG_F(ERROR, "Asset is not loaded: %s id: %i", asset.filepath().c_str(), m_videoAssetId);
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

	/*
	if (input.getKeyState(KeySym::P))
	{
		// This will crash in many cases, because reallocating tables doesn't work correctly.
		m_engine.defragmentTablesAsync(); //RAE_TODO SceneSystem!
	}
	*/

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
			case KeySym::F1:		m_engine.debugSystem().toggleIsEnabled(); break;
			case KeySym::F2:		m_uiSystem.toggleIsEnabled(); break;
			case KeySym::F3:		m_engine.renderSystem().toggleRenderNormals(); break;
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
			// Doesn't quite work right yet. Needs GLFW 3.2 to work properly. possibly.
			//case KeySym::F: m_engine.windowSystem().mainWindow().toggleFullscreen(); break;
			//RAE_OLD case KeySym::Y: m_rayTracer.toggleBufferQuality(); break;
			case KeySym::P: m_engine.rayTracer().toggleFastMode(); break;
			case KeySym::H: m_engine.rayTracer().toggleVisualizeFocusDistance(); break;
			//RAE_OLD case KeySym::_1: m_rayTracer.showScene(1); break;
			//RAE_OLD case KeySym::_2: m_rayTracer.showScene(2); break;
			//RAE_OLD case KeySym::_3: m_rayTracer.showScene(3); break;
			case KeySym::_9:		m_engine.sceneSystem().activeScene().selectNextEntity();
			default:
				break;
		}
	}
}

void Pihlaja::run()
{
	m_engine.run();
}

void Pihlaja::updateDebugTexts()
{
	if (!m_engine.sceneSystem().hasActiveScene())
		return;

	Scene& scene = m_engine.sceneSystem().activeScene();
	auto& transformSystem = scene.transformSystem();
	auto& entitySystem = scene.entitySystem();

	g_debugSystem->showDebugText("");
	g_debugSystem->showDebugText("Scene: " + scene.name());
	g_debugSystem->showDebugText("Esc to quit, F1 Toggle debug info", Colors::white);
	g_debugSystem->showDebugText("Movement: Second mouse button, WASDQE, Arrows", Colors::white);
	g_debugSystem->showDebugText("Raytracer mode: U autofocus, H visualize focus, ", Colors::white);
	g_debugSystem->showDebugText("VB focus distance, NM aperture, KL bounces, ", Colors::white);
	g_debugSystem->showDebugText("G debug view, Tab UI", Colors::white);
	g_debugSystem->showDebugText("Y toggle resolution", Colors::white);
	g_debugSystem->showDebugText("");
	g_debugSystem->showDebugText("Entities on scene: " + std::to_string(entitySystem.entityCount()));
	g_debugSystem->showDebugText("Transforms: " + std::to_string(transformSystem.transformCount()));
	g_debugSystem->showDebugText("Meshes: " + std::to_string(m_assetSystem.meshCount()));
	g_debugSystem->showDebugText("Materials: " + std::to_string(m_assetSystem.materialCount()));
	g_debugSystem->showDebugText("");
}

// OpticalFlow version
UpdateStatus Pihlaja::update()
{
	reactToInput(m_input);

	updateDebugTexts();

#ifdef USE_RAE_AV
	if (!m_play && !m_needsFrameUpdate)
	{
		return UpdateStatus::NotChanged;
	}

	auto& screenImage = m_assetSystem.modifyImage(m_screenImageAssetId);

	if (m_opticalFlow.getState() == EffectNodeState::Nothing ||
		m_opticalFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (!m_avSystem.hasAsset(m_videoAssetId))
		{
			LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (!asset.isLoaded())
		{
			//LOG_F(ERROR, "Asset is not loaded: %s id: %i", asset.filepath().c_str(), m_videoAssetId);
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

	//if (!m_opticalFlow.isDone())
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
	if (!m_play && !m_needsFrameUpdate)
		return UpdateStatus::NotChanged;

	if (m_hdrFlow.getState() == EffectNodeState::Nothing ||
		m_hdrFlow.getState() == EffectNodeState::WaitingForData)
	{
		if (!m_avSystem.hasAsset(m_videoAssetId))
		{
			LOG_F(ERROR, "No asset found in AVSystem with id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		auto& asset = m_avSystem.getAsset(m_videoAssetId);
		if (!asset.isLoaded())
		{
			LOG_F(ERROR, "Asset is not loaded. id: %i", m_videoAssetId);
			return UpdateStatus::NotChanged;
		}

		AVFrame* frameRGB = asset.pullFrame();
		m_frameCount++;

		if (m_videoRenderingState == VideoRenderingState::Player)
		{
			if (m_needsFrameUpdate || (m_evenFrames && m_frameCount % 2 == 0))
				m_avSystem.copyFrameToImage(frameRGB, screenImage);
			else if (m_needsFrameUpdate || (!m_evenFrames && m_frameCount % 2))
				m_avSystem.copyFrameToImage(frameRGB, screenImage);
			m_needsFrameUpdate = false;
		}
		else if (m_videoRenderingState == VideoRenderingState::RenderToScreen ||
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
		if (m_videoRenderingState == VideoRenderingState::RenderToScreen ||
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

	//if (!m_opticalFlow.isDone())
	//{
	//	m_opticalFlow.process();
	//}

	//m_opticalFlow.update(screenImage);

	return (m_needsFrameUpdate || m_play) ? UpdateStatus::Changed : UpdateStatus::NotChanged;
}
*/

