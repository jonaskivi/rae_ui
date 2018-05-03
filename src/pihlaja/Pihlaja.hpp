#pragma once

#include "loguru/loguru.hpp"
#include "rae/animation/Animator.hpp"
#include "rae/visual/Shader.hpp"
#include "rae/scene/Transform.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/Engine.hpp"

#ifdef USE_RAE_AV
#include "rae_av/AVSystem.hpp"
#endif

using namespace rae;
#ifdef USE_RAE_AV
using namespace rae::av;
#endif

enum class VideoRenderingState
{
	Player,
	RenderToScreen,
	RenderToDisk,
};

class Pihlaja : public rae::ISystem
{
public:
	Pihlaja(GLFWwindow* glfwWindow, NVGcontext* nanoVG = nullptr);

	String name() override { return "PihlajaSystem"; }

	Engine* getEngine() { return &m_engine; }

	void initUI();

	void reactToInput(const Input& input);
	void onKeyEvent(const Input& input);

	void run();
	UpdateStatus update() override;
	void destroyEntities(const Array<Id>& entities) override {}
	void defragmentTables() override {}

	void rewind();
	void togglePlay();
	bool isPlay() { return m_play; }

protected:

	void setNeedsFrameUpdate(bool value);

	void setVideoRenderingState(VideoRenderingState state)
	{
		LOG_F(INFO, "Video rendering state: %i", (int)state);
		m_videoRenderingState = state;
		m_needsFrameUpdate = true;
	}

	Engine			m_engine;
	AssetSystem&	m_assetSystem;
	UISystem&		m_uiSystem;

	#ifdef USE_RAE_AV
	AVSystem		m_avSystem;
	#endif

	Input&			m_input;

	asset::Id				m_screenImageAssetId;
	ImageBuffer<uint8_t>	m_screenImage;

	#ifdef USE_RAE_AV
	AssetId					m_videoAssetId;

	HdrFlow					m_hdrFlow;
	OpticalFlow				m_opticalFlow;
	#endif

	VideoRenderingState		m_videoRenderingState = VideoRenderingState::Player;
	bool					m_evenFrames = true;

	int		m_frameCount = 0;
	Bool	m_play = true;
	Bool	m_needsFrameUpdate = true; //false for hdrflow?
};
