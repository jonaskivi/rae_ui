#pragma once

/*JONDE REMOVE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
*/

#include "rae/Random.hpp" //JONDE REMOVE?
#include "rae/Animator.hpp"
#include "rae/Shader.hpp"
#include "rae/ComponentType.hpp"
#include "rae/Entity.hpp"
#include "rae/Transform.hpp"
#include "rae/Mesh.hpp"
#include "rae/Material.hpp"
#include "rae/ObjectFactory.hpp"
#include "rae/System.hpp"
#include "rae/RenderSystem.hpp"
#include "rae/Engine.hpp"

#include "rae_av/AVSystem.hpp"

using namespace rae;
using namespace rae::av;

enum class VideoRenderingState
{
	Player,
	RenderToScreen,
	RenderToDisk,
};

class Pihlaja : public rae::System
{
public:
	Pihlaja(GLFWwindow* glfwWindow);

	Engine* getEngine() { return &m_engine; }

	void initUI();

	void onKeyEvent(const Input& input);

	void run();
	bool update(double time, double deltaTime, std::vector<Entity>&);

protected:

	void setVideoRenderingState(VideoRenderingState state)
	{
		std::cout << "Video rendering state: " << (int)state << "\n";
		m_videoRenderingState = state;
		m_needsFrameUpdate = true;
	}

	Engine m_engine;
	AVSystem m_avSystem;
	Input& m_input;

	ImageBuffer&			m_screenImage;

	AssetId					m_videoAssetId;

	HdrFlow					m_hdrFlow;
	OpticalFlow				m_opticalFlow;

	VideoRenderingState		m_videoRenderingState = VideoRenderingState::Player;
	bool					m_evenFrames = true;

	int m_frameCount = 0;
	bool m_play = true;
	bool m_needsFrameUpdate = false;
};
