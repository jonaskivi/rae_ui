#include "examples/game_menu_example/GameMenuExample.hpp"

#include "rae/ui/UIRenderer.hpp"

using namespace rae;

GameMenuExample::GameMenuExample() :
	m_engine("Game Menu Example", 562, 1000, false),
	m_input(m_engine.input()),
	m_uiSystem(m_engine.uiSystem())
{
	m_engine.addBaseSystems();

	//m_engine.addSystem(m_engine.assetSystem());
	//m_engine.addSystem(m_engine.sceneSystem());

	//m_engine.addSystem(m_engine.editorSystem());
	m_engine.addSystem(m_engine.uiSystem());
	//m_engine.addSystem(m_engine.rayTracer());
	//m_engine.addSystem(m_engine.renderSystem());

	m_engine.addSystem(m_engine.debugSystem());

	/*m_engine.addRenderer3D(m_engine.renderSystem());
	m_engine.addRenderer3D(m_engine.editorSystem());
	m_engine.addRenderer3D(m_engine.debugSystem());
	*/

	m_engine.addRenderer2D(*this);
	m_engine.addRenderer2D(m_engine.uiSystem());
	m_engine.addRenderer2D(m_engine.debugSystem());

	//m_engine.addSystem(m_engine.renderSystem());
	m_engine.addSystem(*this);

	using std::placeholders::_1;
	m_input.connectKeyEventHandler(std::bind(&GameMenuExample::onKeyEvent, this, _1));

	initUI();
}

void GameMenuExample::initUI()
{
	auto& uiSystem = m_uiSystem;
	auto& windowSystem = m_engine.windowSystem();

	UIScene& ui = uiSystem.defaultScene();
	uiSystem.connectWindowToScene(windowSystem.mainWindow(), ui);

	auto& trans = ui.transformSystem();
	auto& anim = ui.animationSystem();

	Id panelParent = ui.createPanel(Rectangle(10.0f, 10.0f, 60.0f, 80.0f));
	trans.addPivot(panelParent, Pivots::TopLeft2D);

	ui.addMaximizer(panelParent);
	ui.toggleMaximizer(panelParent);

	{
		//Id panel = ui.createPanel(Rectangle(10.0f, 10.0f, 60.0f, 80.0f));
		Id panel = ui.createPanel(Rectangle(20.0f, 20.0f, 60.0f, 80.0f));

		ui.addDraggable(panel);
		//ui.addMaximizer(panel);
		//ui.toggleMaximizer(panel);
		//RAE_TEMP ui.addStackLayout(panel);

		Id testButton1 = ui.createButton("Test Button 1",
			Rectangle(10.0f, 10.0f, 22.0f, 6.0f),
			[&, panelParent]()
			{
				LOG_F(INFO, "Activate Test Button 1");
				//windowSystem.createWindow("Test Second Window", 600, 300);
				ui.toggleMaximizer(panelParent);
			});
		ui.addDraggable(testButton1);

		Id testButton2 = ui.createButton("Test Button 2",
			Rectangle(30.0f, 80.0f, 70.0f, 20.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 2");
			});
		trans.addPivot(testButton2, Pivots::Center);

		Id testButton3 = ui.createButton("Anim Button 3",
			Rectangle(10.0f, 30.0f, 60.0f, 10.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Anim Button 3");
			});
		ui.addDraggable(testButton3);

		{
			/*RAE_OLD
			float duration = 2.0f;
			bool isLooping = true;
			anim.addPositionAnimator(testButton3,
				PositionAnimator(
					vec3(10.0f, 30.0f, 0.0f),
					vec3(80.0f, 40.0f, 0.0f),
					duration,
					AnimatorType::CubicOut,
					isLooping));
			*/

			AnimationTimeline& timeline = anim.createAnimationTimeline(0, 120);
			PropertyAnimation<vec3>& posAnim =
				timeline.createVec3Animation(testButton3,
					std::bind(&TransformSystem::getLocalPosition, &trans, std::placeholders::_1),
					std::bind(&TransformSystem::setLocalPosition, &trans, std::placeholders::_1, std::placeholders::_2));

			posAnim.addKeyFrame(0, vec3(10.0f, 30.0f, 0.0f));
			posAnim.addKeyFrame(60, vec3(50.0f, 40.0f, 0.0f));
			posAnim.addKeyFrame(80, vec3(-20.0f, 80.0f, 0.0f));
			posAnim.addKeyFrame(100, vec3(10.0f, 30.0f, 0.0f));
		}

		Id animText = ui.createTextBox("Match Start",
			vec3(), vec3(50.0f, 50.0f, 1.0f), 64.0f);
		ui.addColor(animText, Utils::createColor8bit(54, 168, 175, 255));
		trans.addPivot(animText, Pivots::Center);

		{
			AnimationTimeline& timeline = anim.createAnimationTimeline(0, 200);
			PropertyAnimation<vec3>& posAnim =
				timeline.createVec3Animation(animText,
					std::bind(&TransformSystem::getLocalPosition, &trans, std::placeholders::_1),
					std::bind(&TransformSystem::setLocalPosition, &trans, std::placeholders::_1, std::placeholders::_2));

			posAnim.addKeyFrame(0, vec3(65.0f, -200.0f, 0.0f));
			posAnim.addKeyFrame(50, vec3(65.0f, 120.0f, 0.0f));
			posAnim.addKeyFrame(100, vec3(65.0f, 120.0f, 0.0f));
			posAnim.addKeyFrame(150, vec3(65.0f, 400.0f, 0.0f));
		}

		trans.addChild(panelParent, panel);
		trans.addChild(panel, testButton1);
		trans.addChild(panel, testButton2);
		trans.addChild(panel, testButton3);
	}
}

void GameMenuExample::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			case KeySym::Escape:	m_engine.quit(); break;
			case KeySym::space: break;
			case KeySym::Home: break;
			case KeySym::Tab:
				m_uiSystem.toggleIsEnabled();
				m_engine.debugSystem().toggleIsEnabled();
				break;
			case KeySym::F1:		m_engine.debugSystem().toggleIsEnabled(); break;
			case KeySym::F2:		m_uiSystem.toggleIsEnabled(); break;
			case KeySym::_1: break;
			case KeySym::_2: break;
			case KeySym::R: break;
			default:
				break;
		}
	}
}

void GameMenuExample::run()
{
	m_engine.run();
}

UpdateStatus GameMenuExample::update()
{
	return UpdateStatus::Changed;
}

void GameMenuExample::render2D(UIScene& uiScene, NVGcontext* vg)
{
	if (m_engine.debugSystem().isEnabled())
	{
		// Pixel grid:
		//UIRenderer::renderGrid(vg, 1920.0f, 1080.0f);

		// cm grid:
		UIRenderer::renderGrid(vg, 1920.0f, 1080.0f, m_engine.screenSystem().mmToPixels(1.0f));
	}
}

