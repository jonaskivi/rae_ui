#include "examples/game_menu_example/GameMenuExample.hpp"

#include "rae/ui/UIRenderer.hpp"

using namespace rae;

GameMenuExample::GameMenuExample() :
	m_engine("Game Menu Example", 1920, 1080, false),
	m_input(m_engine.modifyInput()),
	m_uiSystem(m_engine.modifyUiSystem())
{
	m_engine.addBaseSystems();

	//m_engine.addSystem(m_engine.modifyAssetSystem());
	//m_engine.addSystem(m_engine.modifySceneSystem());

	//m_engine.addSystem(m_engine.modifyEditorSystem());
	m_engine.addSystem(m_engine.modifyUiSystem());
	//m_engine.addSystem(m_engine.modifyRayTracer());
	//m_engine.addSystem(m_engine.modifyRenderSystem());

	m_engine.addSystem(m_engine.modifyDebugSystem());

	/*m_engine.addRenderer3D(m_engine.modifyRenderSystem());
	m_engine.addRenderer3D(m_engine.modifyEditorSystem());
	m_engine.addRenderer3D(m_engine.modifyDebugSystem());
	*/

	m_engine.addRenderer2D(*this);
	m_engine.addRenderer2D(m_engine.modifyUiSystem());
	m_engine.addRenderer2D(m_engine.modifyDebugSystem());

	//m_engine.addSystem(m_engine.renderSystem());
	m_engine.addSystem(*this);

	using std::placeholders::_1;
	m_input.connectKeyEventHandler(std::bind(&GameMenuExample::onKeyEvent, this, _1));

	initUI();
}

void GameMenuExample::initUI()
{
	auto& uiSystem = m_uiSystem;
	auto& windowSystem = m_engine.modifyWindowSystem();
	auto& screenSystem = m_engine.screenSystem();

	auto& mainWindow = windowSystem.modifyMainWindow();
	mainWindow.setSize(
		screenSystem.mmToPixels(150.0f),
		screenSystem.mmToPixels(260.0f));

	UIScene& ui = uiSystem.defaultScene();
	uiSystem.connectWindowToScene(mainWindow, ui);

	auto& trans = ui.transformSystem();
	auto& anim = ui.animationSystem();

	Id panelParent = ui.createPanel(Rectangle(10.0f, 10.0f, 60.0f, 90.0f));
	ui.setColor(panelParent, Utils::createColor8bit(43, 47, 59));

	ui.addMaximizer(panelParent);
	ui.toggleMaximizer(panelParent);

	{
		Id panel = ui.createPanel(Rectangle(20.0f, 20.0f, 60.0f, 80.0f));

		ui.addDraggable(panel);

		Id testButton1 = ui.createButton("Test Button 1",
			Rectangle(10.0f, 10.0f, 22.0f, 6.0f),
			[&, panelParent]()
			{
				LOG_F(INFO, "Activate Test Button 1");
				ui.toggleMaximizer(panelParent);
			});
		ui.addDraggable(testButton1);

		Id testButton2 = ui.createButton("Test Button 2",
			Rectangle(30.0f, 60.0f, 50.0f, 12.0f),
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

		trans.addChild(panelParent, panel);
		trans.addChild(panel, testButton1);
		trans.addChild(panel, testButton2);
		trans.addChild(panel, testButton3);

		{
			AnimationTimeline& timeline = anim.createAnimationTimeline(0, 120);
			PropertyAnimation<vec3>& posAnim =
				timeline.createVec3Animation(testButton3,
					std::bind(&TransformSystem::getLocalPosition, &trans, std::placeholders::_1),
					std::bind(&TransformSystem::setLocalPosition, &trans, std::placeholders::_1, std::placeholders::_2));

			posAnim.addKeyFrame(0, vec3(10.0f, 30.0f, 0.0f));
			posAnim.addKeyFrame(60, vec3(50.0f, 40.0f, 0.0f));
			posAnim.addKeyFrame(80, vec3(-20.0f, 80.0f, 0.0f));
			posAnim.addKeyFrame(100, vec3(10.0f, 30.0f, 0.0f));

			PropertyAnimation<Color>& colorAnim =
				timeline.createVec4Animation(testButton3,
					std::bind(&UIScene::getColor, &ui, std::placeholders::_1),
					std::bind(&UIScene::setColor, &ui, std::placeholders::_1, std::placeholders::_2));

			colorAnim.addKeyFrame(0, Color(1.0f, 1.0f, 1.0f, 1.0f));
			colorAnim.addKeyFrame(20, Utils::createColor8bit(250, 95, 46, 225));
			colorAnim.addKeyFrame(60, Color(0.1f, 0.76f, 0.8f, 0.9f));
			colorAnim.addKeyFrame(100, Color(0.1f, 0.76f, 0.8f, 0.2f));
		}

		{
			bool visible = false;
			Id centerPivotContainer = ui.createPanel(Rectangle(10.0f, 10.0f, 60.0f, 80.0f), visible);
			trans.addPivot(centerPivotContainer, Pivots::Center);

			ui.addMaximizer(centerPivotContainer);
			ui.toggleMaximizer(centerPivotContainer);

			Id animText = ui.createTextBox("Match Start",
				vec3(), vec3(50.0f, 50.0f, 1.0f), 64.0f);
			ui.setColor(animText, Colors::white);
			trans.addPivot(animText, Pivots::Center);

			AnimationTimeline& timeline = anim.createAnimationTimeline(0, 200);
			PropertyAnimation<vec3>& posAnim =
				timeline.createVec3Animation(animText,
					std::bind(&TransformSystem::getLocalPosition, &trans, std::placeholders::_1),
					std::bind(&TransformSystem::setLocalPosition, &trans, std::placeholders::_1, std::placeholders::_2));

			posAnim.addKeyFrame(0, vec3(0.0f, -350.0f, 0.0f));
			posAnim.addKeyFrame(50, vec3(0.0f, 0.0f, 0.0f));
			posAnim.addKeyFrame(100, vec3(0.0f, 0.0f, 0.0f));
			posAnim.addKeyFrame(150, vec3(0.0f, 400.0f, 0.0f));

			Id matchRect = ui.createBox(
				Rectangle(0.0f, 0.0f, 50.0f, 20.0f),
				Utils::createColor8bit(231, 6, 78, 0));
			trans.addPivot(matchRect, Pivots::Center);

			// An example of animating a float property with lambdas. The getter is not really used,
			// so we could get rid of that.
			// This would probably be difficult to save into a serialized text file,
			// so most likely we'd need a dedicated Box property animation anyway?
			PropertyAnimation<float>& boxAnim =
				timeline.createFloatAnimation(matchRect,
					[&trans](Id id) -> float
					{
						return trans.getBox(id).width();
					},
					[&trans](Id id, float value)
					{
						trans.modifyBox(id).setWidth(value);
					});

			boxAnim.addKeyFrame(0, 0.0f);
			boxAnim.addKeyFrame(20, 0.0f);
			boxAnim.addKeyFrame(50, 200.0f);

			PropertyAnimation<Color>& colorAnim =
				timeline.createVec4Animation(matchRect,
					std::bind(&UIScene::getColor, &ui, std::placeholders::_1),
					std::bind(&UIScene::setColor, &ui, std::placeholders::_1, std::placeholders::_2));

			colorAnim.addKeyFrame(0, Utils::createColor8bit(231, 6, 78, 0));
			colorAnim.addKeyFrame(20, Utils::createColor8bit(231, 6, 78, 0));
			colorAnim.addKeyFrame(25, Utils::createColor8bit(231, 6, 78, 245));
			colorAnim.addKeyFrame(100, Utils::createColor8bit(231, 6, 78, 235));
			colorAnim.addKeyFrame(150, Utils::createColor8bit(231, 6, 78, 0));

			trans.addChild(centerPivotContainer, matchRect);
			trans.addChild(centerPivotContainer, animText);
		}
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
				m_engine.modifyDebugSystem().toggleIsEnabled();
				break;
			case KeySym::F1:		m_engine.modifyDebugSystem().toggleIsEnabled(); break;
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

