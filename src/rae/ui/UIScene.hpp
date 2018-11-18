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
		DebugSystem& debugSystem);

	TransformSystem&	transformSystem()	{ return m_transformSystem; }

	void handleInput(const Array<InputEvent>& events);
	void viewportHandleInput(const InputState& inputState);
	UpdateStatus update() override;
	void render2D(NVGcontext* nanoVG, const AssetSystem& assetSystem);

	void doLayout();
	void hover();

	// Attach this scene to an existing WindowSystem window.
	Id connectToWindow(const Window& window);
	void updateWindowSize(const Window& window);

	Id createButton(const String& text, std::function<void()> handler);

	Id createButton(const String& text, const Rectangle& rectangle, std::function<void()> handler);
	Id createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler);
	Id createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property);
	Id createTextBox(const String& text, const vec3& position, const vec3& extents);

	int viewportCount() const { return m_viewports.size(); }
	Id createViewport(int sceneIndex, const vec3& position, const vec3& extents);
	void addViewport(Id id, Viewport&& viewport);
	const Viewport& getViewport(Id id) const;
	Viewport& getViewport(Id id);
	Rectangle getViewportPixelRectangle(int sceneIndex) const;
	// Which scene index had events this frame, if any. (-1 for none?)
	int eventsForSceneIndex() { return m_eventsForSceneIndex; }
	// The processed events for this frame in this scene.
	const InputState& inputState() { return m_inputState; }
	void activateViewportForSceneIndex(int sceneIndex);

	Id createPanel(const Rectangle& rectangle);
	Id createPanel(const vec3& position, const vec3& extents);
	void addPanel(Id id, Panel&& panel);
	const Panel& getPanel(Id id);

	void addStackLayout(Id id);

	Id createImageBox(asset::Id imageLink, const vec3& position, const vec3& extents);
	void addImageLink(Id id, ImageLink imageLink);
	const ImageLink& getImageLink(Id id);

	Id createKeyline(Keyline&& element);
	void addKeyline(Id id, Keyline&& element);
	const Keyline& getKeyline(Id id);
	void addKeylineLink(Id childId, Id keylineId); //TODO anchor
	const KeylineLink& getKeylineLink(Id id);

	// RAE_TODO These functions just repeat each other. Possibly all of these should just be functions of the Table
	// and possibly then rename the Table to be a Component class.
	void addText(Id id, const String& text);
	void addText(Id id, Text&& text);
	const Text& getText(Id id);

	void addButton(Id id, Button&& element);
	const Button& getButton(Id id);

	void addColor(Id id, Color&& element);
	const Color& getColor(Id id);

	void addCommand(Id id, Command&& element);
	Command& getCommand(Id id);
	const Command& getCommand(Id id) const;

	void setActive(Id id, bool active);
	bool isActive(Id id);
	// Make active follow property state with a two-way binding
	void bindActive(Id id, Bool& property);

	void addDraggable(Id id);

// internal:

	// Input in millimeters

	// When thickness in mm is 0 use 1 pixel thickness.
	void renderBorder(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color,
		float cornerRadius = 0.0f, float thickness = 0.0f);
	void renderCircle(const Transform& transform, float diameter, const Color& color);
	void renderCircle(const vec2& position, float diameter, const Color& color);
	void renderArc(const vec2& origin, float fromAngleRad, float toAngleRad, float diameter,
		float thickness, const Color& color);
	void renderRectangle(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color);
	void renderButton(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
		const Color& color, const Color& textColor);
	void renderImage(ImageLink imageLink, const Transform& transform, const Box& box, const Pivot& pivot,
		const AssetSystem& assetSystem);

	Rectangle convertToRectangle(const Transform& transform, const Box& box, const Pivot& pivot) const;

private:

	void createDefaultTheme();

	EntitySystem		m_entitySystem;
	TransformSystem		m_transformSystem;
	SelectionSystem		m_selectionSystem;

	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	DebugSystem&		m_debugSystem;

	// The root WindowEntity
	Id					m_rootId = InvalidId;

	Table<WindowEntity>	m_windows;

	Table<Text>			m_texts;
	Table<Button>		m_buttons;
	Array<Color>		m_buttonThemeColors;

	Table<Command>		m_commands;
	Table<Color>		m_colors;
	Table<Active>		m_actives;

	Table<Viewport>		m_viewports;
	Array<Color>		m_viewportThemeColors;
	Table<Panel>		m_panels;
	Array<Color>		m_panelThemeColors;

	Table<Keyline>		m_keylines;
	Table<KeylineLink>	m_keylineLinks;

	Table<StackLayout>	m_stackLayouts;

	Table<ImageLink>	m_imageLinks;

	Table<Draggable>	m_draggables;

	NVGcontext*			m_nanoVG;

	int					m_eventsForSceneIndex = -1;
	// The events processed in this scene during this frame.
	InputState			m_inputState;
};

}
