#include "test/Test2DCoordinates.hpp"

using namespace rae;

Test2DCoordinates::Test2DCoordinates(GLFWwindow* glfwWindow, NVGcontext* nanoVG) :
	m_engine(glfwWindow, nanoVG),
	m_input(m_engine.input()),
	m_uiSystem(m_engine.uiSystem())
{
	m_engine.addSystem(m_engine.input());

	//m_engine.addSystem(m_engine.assetSystem());
	//m_engine.addSystem(m_engine.sceneSystem());

	//m_engine.addSystem(m_engine.editorSystem());
	//m_engine.addSystem(m_engine.uiSystem());
	//m_engine.addSystem(m_engine.rayTracer());
	//m_engine.addSystem(m_engine.renderSystem());

	/*m_engine.addRenderer3D(m_engine.renderSystem());
	m_engine.addRenderer3D(m_engine.editorSystem());
	m_engine.addRenderer3D(m_engine.debugSystem());
	*/

	/////////m_engine.addRenderer2D(m_engine.uiSystem());

	//

	//m_engine.addSystem(m_engine.renderSystem());
	m_engine.addSystem(*this);

	m_engine.addRenderer2D(*this);

	using std::placeholders::_1;
	m_input.connectKeyEventHandler(std::bind(&Test2DCoordinates::onKeyEvent, this, _1));

	initUI();
}

void Test2DCoordinates::initUI()
{
	auto& ui = m_uiSystem;

	int sceneIndex = 0;
	Id viewport = ui.createViewport(sceneIndex,
		virxels(-400.0f, -200.0f, 0.0f),
		virxels(800.0f, 500.0f, 0.1f));

	Id panel = ui.createPanel(
		virxels(-600.0f, 250.0f, 0.0f),
		virxels(250.0f, 325.0f, 0.1f));

	ui.addLayout(panel);
}

void Test2DCoordinates::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			case KeySym::Escape: m_engine.quit(); break;
			case KeySym::space:
			break;
			case KeySym::Home:
			break;
			case KeySym::Tab:
				m_uiSystem.toggleIsEnabled();
				m_engine.debugSystem().toggleIsEnabled();
			break;
			case KeySym::_1:
			break;
			case KeySym::_2:
			break;
			case KeySym::R:
			break;
			default:
			break;
		}
	}
}

void Test2DCoordinates::run()
{
	m_engine.run();
}

UpdateStatus Test2DCoordinates::update()
{
	return UpdateStatus::Changed;
}

void Test2DCoordinates::render2D(NVGcontext* nanoVG)
{
	float cornerRadius = 0.0f;
	m_uiSystem.renderRectangleNano(nanoVG, Rectangle(200.0f, 100.0f, 400.0f, 150.0f),
		cornerRadius, Color(1.0f, 0.0f, 1.0f, 1.0f));

	const float lineHeight = 45.0f;

	nvgFontFace(nanoVG, "sans");

	float vertPos = 250.0f - 100.0f;

	nvgFontSize(nanoVG, 22.0f);
	nvgTextAlign(nanoVG, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

	String text = "Rectangle x: 200 y: 100 w: 400 h: 150";

	nvgFillColor(nanoVG, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
	nvgText(nanoVG, 220.0f, vertPos, text.c_str(), nullptr);
	vertPos += lineHeight;
}

