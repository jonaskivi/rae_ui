#pragma once

#include <vector>

#include <GL/glew.h>

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/ui/Button.hpp"
#include "rae/image/ImageBuffer.hpp"

#include "rae/core/ISystem.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/animation/AnimationSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/WindowSystem.hpp"

#include "rae/ui/UITypes.hpp"

struct NVGcontext;

namespace rae
{

class Time;
class Box;
class Input;
class ScreenSystem;
class AssetSystem;
class DebugSystem;

class UIScene : public ISystem
{
public:
	UIScene(
		const String& name,
		const Time& time,
		Input& input,
		ScreenSystem& screenSystem,
		DebugSystem& debugSystem,
		AssetSystem& assetSystem);

	TransformSystem&		transformSystem() { return m_transformSystem; }
	const TransformSystem&	transformSystem() const { return m_transformSystem; }

	SelectionSystem&		selectionSystem() { return m_selectionSystem; }
	const SelectionSystem&	selectionSystem() const { return m_selectionSystem; }

	AnimationSystem&		animationSystem() { return m_animationSystem; }
	const AnimationSystem&	animationSystem() const { return m_animationSystem; }

	void handleInput(const Array<InputEvent>& events);
	void viewportHandleInput(const InputState& inputState);
	UpdateStatus update() override;
	void render2D(NVGcontext* nanoVG, const AssetSystem& assetSystem);

	void doLayout();
	void hover();
	void hoverText(Id id);

	// Attach this scene to an existing WindowSystem window.
	Id connectToWindow(const Window& window);
	void updateWindowSize(const Window& window);

	Id createButton(const String& text, std::function<void()> handler);

	Id createButton(const String& text, const Rectangle& rectangle, std::function<void()> handler);
	Id createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler);
	Id createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property);
	Id createTextWidget(const String& text, const vec3& position, const vec3& extents, float fontSize = 18.0f,
		bool multiline = false);
	Id createTextBox(
		Text&& text,
		const vec3& position,
		const vec3& extents,
		bool multiline = false);
	Id createBox(const Rectangle& rectangle, const Color& color);
	Id createBox(const vec3& position, const vec3& extents, const Color& color);

	const Table<Viewport>& viewports() const { return m_viewports; }
	int viewportCount() const { return m_viewports.size(); }
	Id createViewport(int sceneIndex, const vec3& position, const vec3& extents);
	Id createAdvancedViewport(int sceneIndex, const vec3& position, const vec3& extents);
	void addViewport(Id id, const Viewport& viewport);
	const Viewport& getViewport(Id id) const;
	Viewport& modifyViewport(Id id);
	Rectangle getViewportPixelRectangle(Id viewportId) const;
	Rectangle getViewportPixelRectangleForSceneIndex(int sceneIndex) const;
	// Which scene index had events this frame, if any. (-1 for none?)
	int eventsForSceneIndex() { return m_eventsForSceneIndex; }
	// The processed events for this frame in this scene.
	const InputState& inputState() { return m_inputState; }
	void activateViewportForSceneIndex(int sceneIndex);

	Id createPanel(const Rectangle& rectangle, bool visible = true);
	Id createPanel(const vec3& position, const vec3& extents, bool visible = true);
	void addPanel(Id id, const Panel& panel);
	const Panel& getPanel(Id id) const;

	void addStackLayout(Id id);
	void addGridLayout(Id id, int xCells = 2, int yCells = 2);

	void addMaximizerAndButton(Id id);
	void addMaximizer(Id id);
	const Maximizer& getMaximizer(Id id) const;
	void toggleMaximizer(Id id);
	void updateMaximizers();

	Id createImageBox(asset::Id imageLink, const vec3& position, const vec3& extents);
	void addImageLink(Id id, ImageLink imageLink);
	const ImageLink& getImageLink(Id id);

	Id createKeyline(const Keyline& element);
	void addKeyline(Id id, const Keyline& element);
	const Keyline& getKeyline(Id id);
	void addKeylineLink(Id childId, Id keylineId); //TODO anchor
	const KeylineLink& getKeylineLink(Id id);

	// RAE_TODO These functions just repeat each other. Possibly all of these should just be functions of the Table
	// and possibly then rename the Table to be a Component class.
	void addText(Id id, const String& text, float fontSize = 18.0f);
	void addText(Id id, const Text& text);
	void addText(Id id, Text&& text);
	const Text& getText(Id id) const;
	Text& modifyText(Id id);

	void addButton(Id id, const Button& element);
	const Button& getButton(Id id) const;
	bool isButton(Id id) const;

	void addColor(Id id, const Color& element);
	void setColor(Id id, const Color& element);
	const Color& getColor(Id id) const;

	void addCommand(Id id, const Command& element);
	const Command& getCommand(Id id) const;
	Command& modifyCommand(Id id);

	void setActive(Id id, bool active);
	bool isActive(Id id) const;
	// Make active follow property state with a two-way binding
	void bindActive(Id id, Bool& property);

	bool isVisible(Id id) const { return ((m_visibles.get(id).visible) == true); }
	void setVisible(Id id, bool visible) { m_visibles.assign(Visible(visible)); }
	void show(Id id)
	{
		if (isVisible(id) == false)
			m_visibles.assign(id, Visible(true));
	}
	void hide(Id id)
	{
		m_visibles.assign(id, Visible(false));
	}

	void addMargin(Id id, const Margin& element);

	void connectUpdater(Id id, std::function<void(Id)> updateFunction);

	void addDraggable(Id id);

	Id rootId() const { return m_rootId; }
	// Also known as windowSize. You can get the window width and height from the rootBox (in mm).
	const Box& rootBox() const { return m_transformSystem.getBox(m_rootId); }

// internal:

	// Input in millimeters

	void renderViewportLine(Id id) const;
	void renderPanel(Id id) const;
	// When thickness in mm is 0 use 1 pixel thickness.
	void renderBorder(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color,
		float cornerRadius = 0.0f, float thickness = 0.0f) const;
	void renderCircle(const Transform& transform, float diameter, const Color& color) const;
	void renderCircle(const vec2& position, float diameter, const Color& color) const;
	void renderArc(const vec2& origin, float fromAngleRad, float toAngleRad, float diameter,
		float thickness, const Color& color) const;
	void renderRectangle(Id id) const;
	void renderRectangleGeneric(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color) const;
	void renderButton(Id id) const;
	void renderButtonGeneric(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
		const Color& color, const Color& textColor) const;
	void renderTextBox(Id id) const;
	void renderTextBoxGeneric(
		const Text& text,
		const TextBox& textBox,
		const Transform& transform,
		const Box& box,
		const Pivot& pivot,
		const Color& color,
		const Color& textColor) const;
	void renderText(Id id) const;
	void renderTextGeneric(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
		float fontSize, const Color& color,
		HorizontalTextAlignment horizontalAlignment = HorizontalTextAlignment::Center,
		VerticalTextAlignment verticalAlignment = VerticalTextAlignment::Center) const;
	void renderMultilineText(Id id) const;
	void renderMultilineTextGeneric(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
		float fontSize, const Color& color, bool limitToBoxWidth = true) const;
	void renderImage(Id id) const;
	void renderImageGeneric(ImageLink imageLink, const Transform& transform, const Box& box, const Pivot& pivot,
		const AssetSystem& assetSystem) const;

	Rectangle convertToPixelRectangle(const Transform& transform, const Box& box, const Pivot& pivot) const;

private:

	void createDefaultTheme();

	// Child systems
	AssetSystem&		m_assetSystem;

	EntitySystem		m_entitySystem;
	TransformSystem		m_transformSystem;
	AnimationSystem		m_animationSystem;
	SelectionSystem		m_selectionSystem;

	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	DebugSystem&		m_debugSystem;

	// The root WindowEntity
	Id					m_rootId = InvalidId;

	Table<WindowEntity>	m_windows;

	Table<UIWidgetRenderer>		m_uiWidgetRenderers;
	Table<UIWidgetUpdater>		m_uiWidgetUpdaters;

	// Widget types (and components that only apply to that widget type).

	Table<TextWidget>	m_textWidgets; // Has a text component and just renders that text without a background or anything.
	Table<TextBox>		m_textBoxes; // Editable textbox.

	Table<Button>		m_buttons; // Clickable button, which usually has a Command that is executed when clicking.
	Array<Color>		m_buttonThemeColors;

	Table<Viewport>		m_viewports; // 3D viewport which is associated with a 3D Scene, and rendered separately from UI.
	Array<Color>		m_viewportThemeColors;

	Table<Panel>		m_panels; // Colored rectangle that usually has a layout and some children.
	Array<Color>		m_panelThemeColors;

	// General purpose shared widget components.

	// Because of Table default value, you should only assign Visibles to entities that might be hidden at some point.
	// So all entities which don't have this assigned, will be visible by default.
	Table<Visible>		m_visibles;
	Table<Text>			m_texts; // General purpose text container for buttons, textboxes and textWidget itself etc.
	Table<Command>		m_commands;
	// RAE_TODO separate foreground color and background color. Otherwise it is ambiguous for Label vs Button.
	Table<Color>		m_colors;
	// Used as general purpose on/off true/false boolean, for example in a CheckBox.
	Table<Active>		m_actives;
	Table<Draggable>	m_draggables;
	Table<ImageLink>	m_imageLinks;

	Table<Margin>		m_margins;
	Table<Keyline>		m_keylines;
	Table<KeylineLink>	m_keylineLinks;

	// Layouts
	Table<StackLayout>	m_stackLayouts;
	Table<GridLayout>	m_gridLayouts;
	bool				m_requestUpdateMaximizers = false;
	Table<Maximizer>	m_maximizers;

	NVGcontext*			m_nanoVG;

	int					m_eventsForSceneIndex = -1;
	// The events processed in this scene during this frame.
	InputState			m_inputState;
};

}
