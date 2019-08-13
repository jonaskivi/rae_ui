#include "test/Test2DCoordinates.hpp"

#include "rae/ui/UIRenderer.hpp"

using namespace rae;

Test2DCoordinates::Test2DCoordinates() :
	m_engine("Test 2D Coordinates", 1920, 1080, false),
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
	// Enable debugSystem to see the grid for this test.
	m_engine.modifyDebugSystem().setIsEnabled(true);

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
	m_input.connectKeyEventHandler(std::bind(&Test2DCoordinates::onKeyEvent, this, _1));

	initUI();
}

void Test2DCoordinates::initUI()
{
	auto& uiSystem = m_uiSystem;
	auto& windowSystem = m_engine.modifyWindowSystem();

	UIScene& ui = uiSystem.defaultScene();
	uiSystem.connectWindowToScene(windowSystem.modifyMainWindow(), ui);

	auto& trans = ui.transformSystem();

	LOG_F(INFO, "Setting sceneIndex to: %i", uiSystem.getSceneIndex(ui));
	LOG_F(INFO, "Set sceneIndex to: %i", windowSystem.mainWindow().uiSceneIndex());
	assert(uiSystem.getSceneIndex(ui) == windowSystem.mainWindow().uiSceneIndex());

	Id keyline1 = ui.createKeyline({ OrientationType::Vertical, 0.3f });

	LOG_F(INFO, "Created keyline id: %i", (int)keyline1);

	int sceneIndex = 0;
	Id viewport = ui.createViewport(sceneIndex,
		vec3(93.0f, 46.0f, 0.0f),
		vec3(186.0f, 116.0f, 1.0f));

	Id testButton0 = ui.createButton("Test Button 0",
			vec3(100.0f, 81.0f, 0.0f),
			vec3(22.0f, 6.0f, 0.1f),
			[&]()
			{
			});

	Id resolutionTextBox = ui.createTextBox("1280x720",
		vec3(80.0f, 15.0f, 0.0f),
		vec3(200.0f, 100.0f, 1.0f),
		44.0f);
	ui.addMaximizer(resolutionTextBox);
	ui.addColor(resolutionTextBox, Color(Colors::gray));
	ui.connectUpdater(resolutionTextBox,
		[&ui](Id id)
		{
			const Box& rootBox = ui.rootBox();

			auto& text = ui.modifyText(id);
			text.text = Utils::toString(rootBox.width()) + "mm x " + Utils::toString(rootBox.height()) + "mm";
		});

	Id testButton05 = ui.createButton("Test Button 0.5",
			Rectangle(100.0f, 121.0f, 26.0f, 6.0f),
			[&]()
			{
			});

	{
		Id panel = ui.createPanel(Rectangle(100.0f, 60.0f, 58.0f, 75.0f));
		ui.addDraggable(panel);

		LOG_F(INFO, "Created panel id: %i", (int)panel);

		ui.addKeylineLink(panel, keyline1);

		ui.addStackLayout(panel);

		//RAE_TODO TESTING: Id keyline2 = ui.createKeyline({ OrientationType::Vertical, 0.1f });
		//trans.addChild(panel, keyline2);

		Id testButton1 = ui.createButton("Test Button 1",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 1");
				//windowSystem.createWindow("Test Second Window", 600, 300);
			});
		ui.addDraggable(testButton1);

		Id testButton2 = ui.createButton("Test Button 2",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 2");
			});
		ui.addDraggable(testButton2);

		Id testButton3 = ui.createButton("Test Button 3",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 3");
			});

		trans.addPivot(testButton3, Pivots::Center);

		trans.addChild(panel, testButton1);
		trans.addChild(panel, testButton2);
		trans.addChild(panel, testButton3);

		//trans.setScale(panel, 0.5f);
	}

	{
		Id panel2 = ui.createPanel(
			vec3(239.0f, 158.0f, 0.0f),
			vec3(58.0f, 75.0f, 1.0f));
		ui.addDraggable(panel2);

		Id testButton4 = ui.createButton("Test Button 4",
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 4");
			});

		Id testButton5 = ui.createButton("Test Button 5",
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 5");
			});

		Id testButton6 = ui.createButton("Test Button 6",
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 6");
			});

		trans.addChild(panel2, testButton4);
		trans.addChild(panel2, testButton5);
		trans.addChild(panel2, testButton6);
	}

	initUISecondWindow();
}

void Test2DCoordinates::initUISecondWindow()
{
	const String windowName = "Test Second Window";

	auto& uiSystem = m_uiSystem;
	auto& windowSystem = m_engine.modifyWindowSystem();

	bool isFullscreen = false;
	Window& window = windowSystem.createWindow(windowName, 600, 300, isFullscreen);

	UIScene& ui = uiSystem.createUIScene(windowName);
	uiSystem.connectWindowToScene(window, ui);

	auto& trans = ui.transformSystem();

	Id keyline1 = ui.createKeyline({ OrientationType::Vertical, 0.5f });

	{
		Id panel = ui.createPanel(Rectangle(100.0f, 60.0f, 58.0f, 75.0f));

		LOG_F(INFO, "Created panel id: %i", (int)panel);

		ui.addKeylineLink(panel, keyline1);

		ui.addStackLayout(panel);

		Id testButton1 = ui.createButton("Create Another Window",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Create Another Window button");
				initUISecondWindow();
			});

		Id testButton2 = ui.createButton("Test Button 8",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 8");
			});

		Id testButton3 = ui.createButton("Test Button 9",
			Rectangle(100.0f, 81.0f, 22.0f, 6.0f),
			[&]()
			{
				LOG_F(INFO, "Activate Test Button 9");
			});

		trans.addPivot(testButton3, Pivots::Center);

		trans.addChild(panel, testButton1);
		trans.addChild(panel, testButton2);
		trans.addChild(panel, testButton3);
	}
}

void Test2DCoordinates::onKeyEvent(const Input& input)
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

void Test2DCoordinates::run()
{
	m_engine.run();
}

UpdateStatus Test2DCoordinates::update()
{
	return UpdateStatus::Changed;
}

void Test2DCoordinates::render2D(UIScene& uiScene, NVGcontext* vg)
{
	if (m_engine.debugSystem().isEnabled())
	{
		// Pixel grid:
		//UIRenderer::renderGrid(vg, 1920.0f, 1080.0f);

		// cm grid:
		UIRenderer::renderGrid(vg, 1920.0f, 1080.0f, m_engine.screenSystem().mmToPixels(1.0f));

		float cornerRadius = 0.0f;
		UIRenderer::renderRectangleNano(vg, Rectangle(200.0f, 100.0f, 400.0f, 150.0f),
			cornerRadius, Color(1.0f, 0.0f, 1.0f, 1.0f));

		const float lineHeight = 45.0f;

		nvgFontFace(vg, "sans");

		float vertPos = 250.0f - 100.0f;

		nvgFontSize(vg, 22.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		String text = "Rectangle x: 200 y: 100 w: 400 h: 150";

		nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgText(vg, 220.0f, vertPos, text.c_str(), nullptr);
		vertPos += lineHeight;
	}
}

