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
	Test2DCoordinates();

	String name() { return "Test2DCoordinates"; }

	Engine* getEngine() { return &m_engine; }

	void initUI();

	void onKeyEvent(const Input& input);

	void run();
	UpdateStatus update() override;
	void render2D(UIScene& uiScene, NVGcontext* nanoVG) override;

	void renderGrid(
		NVGcontext* nanoVG,
		float width = 1920.0f,
		float height = 1080.0f,
		float pixelStep = 10.0f,
		vec2 startPos = vec2(0.0f, 0.0f));

	void destroyEntities(const Array<Id>& entities) override {}
	void defragmentTables() override {}

protected:

	void initUISecondWindow();

	Engine m_engine;
	UISystem& m_uiSystem;
	Input& m_input;

	int		m_frameCount = 0;
	Bool	m_play = true;
};

}
