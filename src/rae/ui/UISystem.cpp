#include "rae/ui/UISystem.hpp"
#include <iostream>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/core/Utils.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "RenderSystem.hpp"

using namespace rae;

static const int ReserveBoxes = 1000;

vec3 rae::virxels(float virtX, float virtY, float virtZ)
{
	static float pixelsToHeight = 1.0f / 1080.0f;
	return vec3(virtX, virtY, virtZ) * pixelsToHeight;
}

vec3 rae::virxels(const vec3& virtualPixels)
{
	static float pixelsToHeight = 1.0f / 1080.0f;
	return virtualPixels * pixelsToHeight;
}

float rae::virxels(float virtualPixels)
{
	static float pixelsToHeight = 1.0f / 1080.0f;
	return virtualPixels * pixelsToHeight;
}

UISystem::UISystem(Input& input, ScreenSystem& screenSystem,
	ObjectFactory& objectFactory, TransformSystem& transformSystem, RenderSystem& renderSystem)
: m_input(input),
m_screenSystem(screenSystem),
m_objectFactory(objectFactory),
m_transformSystem(transformSystem),
m_renderSystem(renderSystem),
m_boxes(ReserveBoxes)
{
	createDefaultTheme();

	m_infoButtonId = createButton("Info",
		virxels(550.0f, 300.0f, 0.0f),
		virxels(300.0f, 25.0f, 0.1f),
		[](){});
}

void UISystem::createDefaultTheme()
{
	m_buttonThemeColours.resize((size_t)ButtonThemeColourKey::Count);
	m_buttonThemeColours[(size_t)ButtonThemeColourKey::Background]	= Colour(0.1f, 0.1f, 0.1f, 1.0f);
	m_buttonThemeColours[(size_t)ButtonThemeColourKey::Hover]		= Colour(0.0f, 1.0f, 1.0f, 1.0f);
	m_buttonThemeColours[(size_t)ButtonThemeColourKey::Active]		= Colour(0.1f, 0.723f, 0.235f, 1.0f);
	m_buttonThemeColours[(size_t)ButtonThemeColourKey::ActiveHover]	= Colour(0.1f, 1.0f, 0.235f, 1.0f);
}

bool UISystem::update(double time, double deltaTime, Array<Entity>& entities)
{
	static int frameCount = 0;

	// hover boxes
	for (auto& entity : entities)
	{
		Id id = entity.id();
		if (m_transformSystem.hasTransform(id)
			&& m_boxes.check(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			Box tbox = m_boxes.get(id);
			tbox.transform(transform);

			if (tbox.hit(vec2(m_input.mouse.x, m_input.mouse.y)))
			{
				setHovered(id, true);

				if (m_commands.check(id))
				{
					if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonRelease)
					{
						auto& command = getCommand(id);
						command.execute();
					}
				}
			}
			else
			{
				setHovered(id, false);
			}
		}
	}

	// debug rendering
	if (m_buttons.check(m_infoButtonId))
	{
		auto& button = m_buttons.get(m_infoButtonId);
		button.setText("Mouse: xP: " + Utils::toString(m_input.mouse.xP)
			+ " yP: " + Utils::toString(m_input.mouse.yP)
			+ " x: " + Utils::toString(m_input.mouse.x)
			+ " y: " + Utils::toString(m_input.mouse.y)
			+ " frame: " + Utils::toString(frameCount));
	}

	/* TEMP DEBUG
	if (m_transformSystem.hasTransform(m_infoButtonId))
	{
		auto& transform = m_transformSystem.getTransform(m_infoButtonId);
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			transform.setTarget(vec3(m_input.mouse.x, m_input.mouse.y, 0.0f), 1.0f);
			std::cout << "Click " << frameCount << "\n";
		}
	}*/

	frameCount++;

	return false;
}

void UISystem::render(double time, double deltaTime, Array<Entity>& entities, NVGcontext* vg,
	int windowWidth, int windowHeight, float screenPixelRatio)
{
	m_vg = vg;

	const Colour& buttonBackgroundColour = m_buttonThemeColours[(size_t)ButtonThemeColourKey::Background];
	const Colour& buttonHoverColour = m_buttonThemeColours[(size_t)ButtonThemeColourKey::Hover];
	const Colour& buttonActiveColour = m_buttonThemeColours[(size_t)ButtonThemeColourKey::Active];
	const Colour& buttonActiveHoverColour = m_buttonThemeColours[(size_t)ButtonThemeColourKey::ActiveHover];

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(vg, windowWidth, windowHeight, screenPixelRatio);

		int i = 0;
		for (auto& entity : entities)
		{
			Id id = entity.id();
			if (m_transformSystem.hasTransform(id) and
				m_buttons.check(id) and
				m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = getBox(id);
				const Button& button = getButton(id);
				bool hasColour = m_colours.check(id);
				bool active = isActive(id);
				bool hovered = m_hovers.check(id);

				renderButton(button.text(), transform, box,
					(hovered and active ? buttonActiveHoverColour :
						hovered ? buttonHoverColour :
						active ? buttonActiveColour :
						hasColour ? getColour(id) :
						buttonBackgroundColour));
			}
			else if (m_transformSystem.hasTransform(id) and
				m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = getBox(id);
				bool hasColour = m_colours.check(id);
				bool active = isActive(id);
				bool hovered = m_hovers.check(id);

				renderButton(getText(id), transform, box,
					(hovered && active ? buttonActiveHoverColour :
						hovered ? buttonHoverColour :
						active ? buttonActiveColour :
						hasColour ? getColour(id) :
						buttonBackgroundColour));
			}
		}

		//JONDE nvgFontFace(vg, "sans");
		nvgFontFace(vg, "logo");

		float vertPos = 10.0f;

		nvgFontSize(vg, 18.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
		nvgText(vg, 10.0f, vertPos, "fps_here", nullptr); vertPos += 20.0f;

		nvgText(vg, 10.0f, vertPos, "HELLO WORLD. Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
			" NM aperture, KL bounces, G debug view, T text, U fastmode", nullptr); vertPos += 20.0f;
		nvgText(vg, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
		nvgText(vg, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

		std::string entity_count_str = "Entities: " + std::to_string(m_objectFactory.entityCount());
		nvgText(vg, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string transform_count_str = "Transforms: " + std::to_string(m_transformSystem.transformCount());
		nvgText(vg, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string mesh_count_str = "Meshes: " + std::to_string(m_objectFactory.meshCount());
		nvgText(vg, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string material_count_str = "Materials: " + std::to_string(m_objectFactory.materialCount());
		nvgText(vg, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

		//nvgText(vg, 10.0f, vertPos, m_pickedString.c_str(), nullptr);

	nvgEndFrame(vg);
}

void UISystem::renderButton(const String& text, const Transform& transform, const Box& box, const Colour& colour)
{
	vec3 dimensions = box.dimensions();
	float halfWidth = dimensions.x * 0.5f;
	float halfHeight = dimensions.y * 0.5f;

	renderButtonNano(m_vg, text,
		m_screenSystem.heightToPixels(transform.position.x - halfWidth) + (m_renderSystem.windowPixelWidth() * 0.5f),
		m_screenSystem.heightToPixels(transform.position.y - halfHeight) + (m_renderSystem.windowPixelHeight() * 0.5f),
		m_screenSystem.heightToPixels(dimensions.x),
		m_screenSystem.heightToPixels(dimensions.y),
		m_screenSystem.heightToPixels(virxels(2.0f)), // cornerRadius
		colour
		);
}

void UISystem::renderWindowNano(NVGcontext* vg, const String& title, float x, float y, float w, float h,
							float cornerRadius)
{
	//float cornerRadius = 30.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// no negative windows please:
	if(w < 30.0f) w = 30.0f;
	if(h < 30.0f) h = 30.0f;

	nvgSave(vg);
//	nvgClearState(vg);

	// Window

	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBA(255,255,255,135), nvgRGBA(0,0,0,135));
	//JONDE headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBAf(1.0f,1.0f,1.0f, a()), nvgRGBAf(r(),g(),b(),a()));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	//nvgFillColor(vg, nvgRGBA(28,30,34,192));
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	//nvgFill(vg);

	//nvgFillColor(vg, nvgRGBA(155,155,155,255));
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	//JONDE nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f * a()));
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	// Drop shadow
	//JONDE shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x-60,y-60, w+120,h+120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);
/*
	// Header
	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBA(255,255,255,8), nvgRGBA(0,0,0,16));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x+1,y+1, w-2,30, cornerRadius-1);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	nvgBeginPath(vg);
	nvgMoveTo(vg, x+0.5f, y+0.5f+30);
	nvgLineTo(vg, x+0.5f+w-1, y+0.5f+30);
	nvgStrokeColor(vg, nvgRGBA(0,0,0,32));
	nvgStroke(vg);
*/
	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Shadow
	nvgFontBlur(vg,2);
	//JONDE nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()));
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, x+w/2,y+16+1, title.c_str(), nullptr);

	// Actual title text
	nvgFontBlur(vg,0);
	//textcolor:
	//nvgFillColor(vg, nvgRGBA(220,220,220,160));
	//JONDE nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,a()));
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, x+w/2,y+16, title.c_str(), nullptr);

	nvgRestore(vg);
}

void UISystem::renderButtonNano(NVGcontext* vg, const String& text, float x, float y, float w, float h,
							float cornerRadius, const Colour& colour)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// no negative buttons please:
	if(w < 5.0f) w = 5.0f;
	if(h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, x,y,x,y+15,
		nvgRGBAf(colour.r, colour.g, colour.b, colour.a),
		nvgRGBAf(colour.r, colour.g, colour.b, colour.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	/*
	// Outline
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);
	*/

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x-60,y-60, w+120,h+120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, x+w/2,y+16+1, text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, x+w/2,y+16, text.c_str(), nullptr);

	nvgRestore(vg);
}

Id UISystem::createButton(const String& text, vec3 position, vec3 extents, std::function<void()> handler)
{
	Entity& entity = m_objectFactory.createEmptyEntity();
	//m_transformSystem.addTransform(entity.id(), Transform(vec3(0.0f, 0.0f, 0.0f)));
	m_transformSystem.addTransform(entity.id(), Transform(position));
	//m_transformSystem.setPosition(entity.id(), position); //JONDE TEMP animation

	vec3 halfExtents = extents / 2.0f;
	addBox(entity.id(), Box(-(halfExtents), halfExtents));
	addButton(entity.id(), Button(text));
	addCommand(entity.id(), Command(handler));
	return entity.id();
}

Id UISystem::createTextBox(const String& text, vec3 position, vec3 extents)
{
	Entity& entity = m_objectFactory.createEmptyEntity();
	m_transformSystem.addTransform(entity.id(), Transform(position));
	m_transformSystem.setPosition(entity.id(), position);

	vec3 halfExtents = extents / 2.0f;
	addBox(entity.id(), Box(-(halfExtents), halfExtents));
	addText(entity.id(), text);
	return entity.id();
}

void UISystem::addBox(Id id, Box&& box)
{
	m_boxes.create(id, std::move(box));
}

const Box& UISystem::getBox(Id id)
{
	return m_boxes.get(id);
}

void UISystem::addText(Id id, const String& text)
{
	m_texts.create(id, std::move(Text(text)));
}

void UISystem::addText(Id id, Text&& text)
{
	m_texts.create(id, std::move(text));
}

const Text& UISystem::getText(Id id)
{
	return m_texts.get(id);
}

void UISystem::addButton(Id id, Button&& element)
{
	m_buttons.create(id, std::move(element));
}

const Button& UISystem::getButton(Id id)
{
	return m_buttons.get(id);
}

void UISystem::addColour(Id id, Colour&& element)
{
	m_colours.create(id, std::move(element));
}

const Colour& UISystem::getColour(Id id)
{
	return m_colours.get(id);
}

void UISystem::addCommand(Id id, Command&& element)
{
	m_commands.create(id, std::move(element));
}

const Command& UISystem::getCommand(Id id)
{
	return m_commands.get(id);
}

void UISystem::setHovered(Id id, bool hovered)
{
	if (hovered)
		m_hovers.create(id, std::move(Hover()));
	else m_hovers.remove(id);
}

bool UISystem::isHovered(Id id)
{
	return m_hovers.check(id);
}

void UISystem::setActive(Id id, bool active)
{
	m_actives.create(id, std::move(Active(active)));
}

bool UISystem::isActive(Id id)
{
	return m_actives.get(id);
}
