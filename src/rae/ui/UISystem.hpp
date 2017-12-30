#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
using glm::vec3;

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/visual/Box.hpp"
#include "rae/ui/Button.hpp"

#include "rae/core/ISystem.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/visual/TransformSystem.hpp"

struct NVGcontext;

namespace rae
{

class Input;
class ScreenSystem;
class RenderSystem;

vec3 virxels(float virtX, float virtY, float virtZ);
vec3 virxels(const vec3& virtualPixels);
float virxels(float virtualPixels);

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

//JONDE TODO Move to ButtonSubsystem
enum class ButtonThemeColourKey
{
	Background,
	Hover,
	Active,
	ActiveHover,
	Count
};

class UISystem : public ISystem
{
public:
	UISystem(Input& input, ScreenSystem& screenSystem,
		EntitySystem& entitySystem, TransformSystem& transformSystem, RenderSystem& renderSystem);

	String name() override { return "UISystem"; }

	bool update(double time, double deltaTime) override;
	void destroyEntities(const Array<Id>& entities) override;
	void defragmentTables() override;
	void render(double time, double deltaTime, NVGcontext* vg,
				int windowWidth, int windowHeight, float screenPixelRatio);

	Id createButton(const String& text, vec3 position, vec3 extents, std::function<void()> handler);
	Id createTextBox(const String& text, vec3 position, vec3 extents);

	void addBox(Id id, Box&& box);
	const Box& getBox(Id id);

	void addText(Id id, const String& text);
	void addText(Id id, Text&& text);
	const Text& getText(Id id);

	void addButton(Id id, Button&& element);
	const Button& getButton(Id id);

	void addColour(Id id, Colour&& element);
	const Colour& getColour(Id id);

	void addCommand(Id id, Command&& element);
	const Command& getCommand(Id id);

	void setHovered(Id id, bool hovered);
	bool isHovered(Id id);

	void setActive(Id id, bool active);
	bool isActive(Id id);

private:

	void createDefaultTheme();

	void renderButton(const String& text, const Transform& transform, const Box& box, const Colour& colour);

	// NanoVG takes input in pixels, and so do these helper functions:
	void renderWindowNano(NVGcontext* vg, const String& title, float x, float y, float w, float h,
					  float cornerRadius);
	void renderButtonNano(NVGcontext* vg, const String& text, float x, float y, float w, float h,
					  float cornerRadius, const Colour& colour = Colour(0.1f, 0.1f, 0.1f, 1.0f));

	Input& m_input;
	/*TODO const*/ ScreenSystem& m_screenSystem;
	EntitySystem& m_entitySystem;
	TransformSystem& m_transformSystem;
	RenderSystem& m_renderSystem;
	NVGcontext* m_vg;

	Id m_infoButtonId;

	Table<Box> m_boxes;
	Table<Text> m_texts;
	Table<Button> m_buttons;
	Array<Colour> m_buttonThemeColours;
	Table<Command> m_commands;
	Table<Colour> m_colours;
	Table<Active> m_actives;
	Table<Hover> m_hovers;
};

}
