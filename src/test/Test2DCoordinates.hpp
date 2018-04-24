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

namespace rae
{

class Test2DCoordinates : public rae::ISystem
{
public:
	Test2DCoordinates(GLFWwindow* glfwWindow, NVGcontext* nanoVG = nullptr);

	String name() { return "Test2DCoordinates"; }

	Engine* getEngine() { return &m_engine; }

	void initUI();

	void onKeyEvent(const Input& input);

	void run();
	UpdateStatus update() override;
	void render2D(NVGcontext* nanoVG) override;

	void destroyEntities(const Array<Id>& entities) override {}
	void defragmentTables() override {}

protected:

	Engine m_engine;
	UISystem& m_uiSystem;
	Input& m_input;

	int		m_frameCount = 0;
	Bool	m_play = true;
};

}
