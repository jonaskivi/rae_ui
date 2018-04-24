#pragma once

#include <vector>

#include <GL/glew.h>

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/visual/Box.hpp"
#include "rae/ui/Button.hpp"
#include "rae/image/ImageBuffer.hpp"

#include "rae/core/ISystem.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/scene/TransformSystem.hpp"

struct NVGcontext;

namespace rae
{

class Input;
class ScreenSystem;
class AssetSystem;
class DebugSystem;

vec3 virxels(float virtX, float virtY, float virtZ);
vec3 virxels(const vec3& virtualPixels);
float virxels(float virtualPixels);

struct Rectangle
{
	Rectangle(){}
	Rectangle(float x, float y, float width, float height) :
		x(x), y(y), width(width), height(height)
	{}

	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
};

struct Layout
{
	Array<Id> children;
};

struct Hover
{
};

struct Active
{
	Active(){}
	Active(bool active) :
		active(active)
	{
	}

	operator bool&() { return active; }
	operator bool() const { return active; }

	bool active = false;
};

struct Text
{
	Text(){}
	Text(const String& text) :
		text(text)
	{
	}

	operator String&() { return text; }
	operator String() const { return text; }

	String text;
};

struct Panel
{
};

struct Viewport
{
	Viewport(){}
	Viewport(int sceneIndex) :
		sceneIndex(sceneIndex)
	{}

	int sceneIndex = 0;
};

using ImageLink = asset::Id;

// RAE_TODO Move to ButtonSubsystem
enum class ButtonThemeColorKey
{
	Background,
	Hover,
	Active,
	ActiveHover,
	Text,
	HoverText,
	ActiveText,
	ActiveHoverText,
	Count
};

enum class PanelThemeColorKey
{
	Background,
	Hover,
	Count
};

class UISystem : public ISystem
{
public:
	UISystem(
		const Time& time,
		Input& input,
		ScreenSystem& screenSystem,
		AssetSystem& assetSystem,
		DebugSystem& debugSystem);

	~UISystem();

	String name() override { return "UISystem"; }

	UpdateStatus update() override;
	virtual void render2D(NVGcontext* nanoVG) override;

	void doLayout();
	void hover();

	Id createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler);
	Id createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property);
	Id createTextBox(const String& text, const vec3& position, const vec3& extents);

	int viewportCount() { return m_viewports.size(); }
	Id createViewport(int sceneIndex, const vec3& position, const vec3& extents);
	void addViewport(Id id, Viewport&& viewport);
	const Viewport& getViewport(Id id);
	Rectangle getViewportPixelRectangle(int sceneIndex);

	Id createPanel(const vec3& position, const vec3& extents);
	void addPanel(Id id, Panel&& panel);
	const Panel& getPanel(Id id);

	void addLayout(Id id);
	void addToLayout(Id layoutId, Id childId);

	Id createImageBox(asset::Id imageLink, const vec3& position, const vec3& extents);
	void addImageLink(Id id, ImageLink&& imageLink);
	const ImageLink& getImageLink(Id id);

	// RAE_TODO These functions just repeat each other. Possibly all of these should just be functions of the Table
	// and possibly then rename the Table to be a Component class.
	void addBox(Id id, Box&& box);
	const Box& getBox(Id id);

	void addText(Id id, const String& text);
	void addText(Id id, Text&& text);
	const Text& getText(Id id);

	void addButton(Id id, Button&& element);
	const Button& getButton(Id id);

	void addColor(Id id, Color&& element);
	const Color& getColor(Id id);

	void addCommand(Id id, Command&& element);
	const Command& getCommand(Id id);

	void setHovered(Id id, bool hovered);
	bool isHovered(Id id);

	void setActive(Id id, bool active);
	bool isActive(Id id);
	// Make active follow property state with a two-way binding
	void bindActive(Id id, Bool& property);

// internal:

	void renderRectangle(const Transform& transform, const Box& box, const Color& color);
	void renderButton(const String& text, const Transform& transform, const Box& box,
		const Color& color, const Color& textColor);
	void renderBorder(const Transform& transform, const Box& box, const Color& color);
	void renderImage(ImageLink imageLink, const Transform& transform, const Box& box);

	Rectangle convertToRectangle(const Transform& transform, const Box& box) const;

	// NanoVG takes input in pixels, and so do these helper functions:
	void renderBorderNano(NVGcontext* vg, const Rectangle& rectangle,
			float cornerRadius,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderRectangleNano(NVGcontext* vg, const Rectangle& rectangle,
			float cornerRadius,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderWindowNano(NVGcontext* vg, const String& title, const Rectangle& rectangle,
			float cornerRadius, const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderButtonNano(NVGcontext* vg, const String& text, const Rectangle& rectangle,
			float cornerRadius,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f),
			const Color& textColor = Color(1.0f, 1.0f, 1.0f, 1.0f));

private:

	void createDefaultTheme();

	EntitySystem		m_entitySystem;
	TransformSystem		m_transformSystem;

	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	AssetSystem&		m_assetSystem;
	DebugSystem&		m_debugSystem;
	NVGcontext*			m_nanoVG;

	Id					m_infoButtonId;

	Table<Box>			m_boxes;
	Table<Text>			m_texts;
	Table<Button>		m_buttons;
	Array<Color>		m_buttonThemeColors;

	Table<Command>		m_commands;
	Table<Color>		m_colors;
	Table<Active>		m_actives;
	Table<Hover>		m_hovers;

	Table<Viewport>		m_viewports;
	Table<Panel>		m_panels;
	Array<Color>		m_panelThemeColors;

	Table<Layout>		m_layouts;

	Table<ImageLink>	m_imageLinks;
};

extern UISystem* g_ui;

}
