#include "rae/ui/UISystem.hpp"
#include <iostream>
#include <ciso646>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/core/Log.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/visual/RenderSystem.hpp"

using namespace rae;

static const int ReserveBoxes = 1000;

const float VirtualPixelsBase = 1080.0f;
const float VirtualPixelsFactor = 1.0f / VirtualPixelsBase;

vec3 rae::virxels(float virtX, float virtY, float virtZ)
{
	//static float pixelsToHeight = 1.0f / VirtualPixelsBase;
	return vec3(virtX, virtY, virtZ) * VirtualPixelsFactor;
}

vec3 rae::virxels(const vec3& virtualPixels)
{
	return virtualPixels * VirtualPixelsFactor;
}

float rae::virxels(float virtualPixels)
{
	return virtualPixels * VirtualPixelsFactor;
}

UISystem::UISystem(Input& input, ScreenSystem& screenSystem,
	EntitySystem& entitySystem, TransformSystem& transformSystem, RenderSystem& renderSystem) :
		m_input(input),
		m_screenSystem(screenSystem),
		m_entitySystem(entitySystem),
		m_transformSystem(transformSystem),
		m_renderSystem(renderSystem),
		m_boxes(ReserveBoxes)
{
	addTable(m_boxes);
	addTable(m_texts);
	addTable(m_buttons);
	addTable(m_commands);
	addTable(m_colors);
	addTable(m_actives);
	addTable(m_hovers);
	addTable(m_panels);
	addTable(m_layouts);

	createDefaultTheme();

	m_infoButtonId = createButton("Info",
		virxels(-550.0f, -300.0f, 0.0f),
		virxels(300.0f, 25.0f, 0.1f),
		[](){});

	//rae_log("UISystem creating Info button: ", m_infoButtonId);
}

void UISystem::createDefaultTheme()
{
	m_buttonThemeColors.resize((size_t)ButtonThemeColorKey::Count);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Background]		= Color(0.1f, 0.1f, 0.1f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Hover]				= Color(0.3f, 0.4f, 0.45f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Active]			= Color(0.0f, 0.921f, 0.862f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHover]		= Color(0.619f, 1.0f, 0.976f, 1.0f);

	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Text]				= Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::HoverText]			= Color(0.0f, 0.0f, 0.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveText]		= Color(0.0f, 0.0f, 0.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHoverText]	= Color(0.0f, 0.0f, 0.0f, 1.0f);

	m_panelThemeColors.resize((size_t)PanelThemeColorKey::Count);
	m_panelThemeColors[(size_t)PanelThemeColorKey::Background]	= Utils::createColor8bit(52, 61, 70, 255);
	m_panelThemeColors[(size_t)PanelThemeColorKey::Hover]			= Utils::createColor8bit(52, 61, 70, 255);
}

UpdateStatus UISystem::update()
{
	static int frameCount = 0;

	doLayout();

	hover();

	// debug rendering
	if (m_buttons.check(m_infoButtonId))
	{
		auto& button = m_buttons.get(m_infoButtonId);
		const auto& transform = m_transformSystem.getTransform(m_infoButtonId);
		button.setText("Mouse: xP: " + Utils::toString(m_input.mouse.xP)
			+ " yP: " + Utils::toString(m_input.mouse.yP)
			+ " x: " + Utils::toString(m_input.mouse.x)
			+ " y: " + Utils::toString(m_input.mouse.y)
			+ " frame: " + Utils::toString(frameCount)
			+ "\nx: " + Utils::toString(transform.position.x)
			+ "y: " + Utils::toString(transform.position.y)
			+ "z: " + Utils::toString(transform.position.z)
			);
	}

	// TEMP DEBUG
	/*
	if (m_transformSystem.hasTransform(m_infoButtonId))
	{
		auto& transform = m_transformSystem.getTransform(m_infoButtonId);
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			//rae_log("UISystem::render settings stuff mouse.x: ",
			//	m_input.mouse.x, " mouse.y: ", m_input.mouse.y);

			transform.setTarget(vec3(m_input.mouse.x, m_input.mouse.y, 0.0f), 1.0f);
			//rae_log("Click ", frameCount);
		}
	}
	*/

	frameCount++;

	return UpdateStatus::NotChanged;
}

void UISystem::doLayout()
{
	query<Layout>(m_layouts, [&](Id layoutId, Layout& layout)
	{
		const vec3& parentPos = m_transformSystem.getPosition(layoutId);
		const auto& parentBox = m_boxes.get(layoutId);

		float margin = virxels(24.0f);
		float someValue = virxels(30.0f);
		float someIter = parentPos.y + parentBox.min().y;
		for (auto&& childId : layout.children)
		{
			vec3 pos = m_transformSystem.getPosition(childId);
			const auto& box = m_boxes.get(childId);

			pos.x = (parentPos.x + parentBox.min().x) - box.min().x + margin;
			pos.y = someIter - box.min().y + margin;
			m_transformSystem.setPosition(childId, pos);
			someIter = someIter + someValue;
		}
	});
}

void UISystem::hover()
{
	// hover boxes
	for (Id id : m_entitySystem.entities())
	{
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
}

void UISystem::render2D(NVGcontext* nanoVG)
{
	const auto& window = m_screenSystem.window();
	int windowWidth = window.width();
	int windowHeight = window.height();
	float screenPixelRatio = window.screenPixelRatio();

	m_nanoVG = nanoVG;

	const Color& buttonBackgroundColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Background];
	const Color& buttonHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Hover];
	const Color& buttonActiveColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Active];
	const Color& buttonActiveHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHover];

	const Color& buttonTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Text];
	const Color& buttonHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::HoverText];
	const Color& buttonActiveTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveText];
	const Color& buttonActiveHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHoverText];

	const Color& panelBackgroundColor = m_panelThemeColors[(size_t)PanelThemeColorKey::Background];
	const Color& panelHoverColor = m_panelThemeColors[(size_t)PanelThemeColorKey::Hover];

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(nanoVG, windowWidth, windowHeight, screenPixelRatio);

		int i = 0;
		for (Id id : m_entitySystem.entities())
		{
			if (m_buttons.check(id) and
				m_transformSystem.hasTransform(id) and
				m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = getBox(id);
				const Button& button = getButton(id);
				bool hasColor = m_colors.check(id);
				bool active = isActive(id);
				bool hovered = m_hovers.check(id);

				renderButton(button.text(), transform, box,
					(hovered and active ? buttonActiveHoverColor :
						hovered ? buttonHoverColor :
						active ? buttonActiveColor :
						hasColor ? getColor(id) :
						buttonBackgroundColor),
					(hovered and active ? buttonActiveHoverTextColor :
						hovered ? buttonHoverTextColor :
						active ? buttonActiveTextColor :
						buttonTextColor));
			}
			else if (m_panels.check(id) and
					 m_transformSystem.hasTransform(id) and
					 m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = getBox(id);
				bool hasColor = m_colors.check(id);

				bool hovered = m_hovers.check(id);

				renderRectangle(transform, box,
						hovered ? panelHoverColor :
						hasColor ? getColor(id) :
						panelBackgroundColor);
			}
			// TODO this last case is strange
			else if (m_transformSystem.hasTransform(id) and
					 m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = getBox(id);
				bool hasColor = m_colors.check(id);
				bool active = isActive(id);
				bool hovered = m_hovers.check(id);

				renderButton(getText(id), transform, box,
					(hovered and active ? buttonActiveHoverColor :
						hovered ? buttonHoverColor :
						active ? buttonActiveColor :
						hasColor ? getColor(id) :
						buttonBackgroundColor),
					(hovered and active ? buttonActiveHoverTextColor :
						hovered ? buttonHoverTextColor :
						active ? buttonActiveTextColor :
						buttonTextColor));
			}

		}

		nvgFontFace(nanoVG, "sans");
		//nvgFontFace(nanoVG, "logo");

		float vertPos = 10.0f;

		nvgFontSize(nanoVG, 18.0f);
		nvgTextAlign(nanoVG, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(nanoVG, nvgRGBA(128, 128, 128, 192));
		nvgText(nanoVG, 10.0f, vertPos, m_renderSystem.fpsString().c_str(), nullptr); vertPos += 20.0f;

		nvgText(nanoVG, 10.0f, vertPos, "HELLO WORLD. Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
			" NM aperture, KL bounces, G debug view, Tab UI, U fastmode", nullptr); vertPos += 20.0f;
		nvgText(nanoVG, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
		nvgText(nanoVG, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

		std::string entity_count_str = "Entities: " + std::to_string(m_entitySystem.entityCount());
		nvgText(nanoVG, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string transform_count_str = "Transforms: " + std::to_string(m_transformSystem.transformCount());
		nvgText(nanoVG, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

		//std::string mesh_count_str = "Meshes: " + std::to_string(m_assetSystem.meshCount());
		//nvgText(nanoVG, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

		//std::string material_count_str = "Materials: " + std::to_string(m_assetSystem.materialCount());
		//nvgText(nanoVG, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

		//nvgText(nanoVG, 10.0f, vertPos, m_pickedString.c_str(), nullptr);

	nvgEndFrame(nanoVG);
}

void UISystem::renderRectangle(const Transform& transform, const Box& box, const Color& color)
{
	vec3 dimensions = box.dimensions();
	float halfWidth = dimensions.x * 0.5f;
	float halfHeight = dimensions.y * 0.5f;

	const auto& window = m_screenSystem.window();

	renderRectangleNano(m_nanoVG,
		m_screenSystem.heightToAltPixels(transform.position.x - halfWidth) + (window.width() * 0.5f),
		m_screenSystem.heightToAltPixels(transform.position.y - halfHeight) + (window.height() * 0.5f),
		m_screenSystem.heightToAltPixels(dimensions.x),
		m_screenSystem.heightToAltPixels(dimensions.y),
		0.0f, // cornerRadius
		color);
}

void UISystem::renderButton(const String& text, const Transform& transform, const Box& box,
	const Color& color, const Color& textColor)
{
	vec3 dimensions = box.dimensions();
	float halfWidth = dimensions.x * 0.5f;
	float halfHeight = dimensions.y * 0.5f;

	const auto& window = m_screenSystem.window();

	renderButtonNano(m_nanoVG, text,
		m_screenSystem.heightToAltPixels(transform.position.x - halfWidth) + (window.width() * 0.5f),
		m_screenSystem.heightToAltPixels(transform.position.y - halfHeight) + (window.height() * 0.5f),
		m_screenSystem.heightToAltPixels(dimensions.x),
		m_screenSystem.heightToAltPixels(dimensions.y),
		m_screenSystem.heightToAltPixels(virxels(2.0f)), // cornerRadius
		color,
		textColor);
}

void UISystem::renderRectangleNano(NVGcontext* vg, float x, float y, float w, float h,
	float cornerRadius, const Color& color)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negatives please:
	if(w < 5.0f) w = 5.0f;
	if(h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, x, y, x, y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, x, y+5, w,h, cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x - 60, y - 60, w + 120, h + 120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void UISystem::renderWindowNano(NVGcontext* vg, const String& title, float x, float y, float w, float h,
							float cornerRadius, const Color& color)
{
	//float cornerRadius = 30.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative windows please:
	if (w < 30.0f)
	{
		w = 30.0f;
	}

	if (h < 30.0f)
	{
		h = 30.0f;
	}

	nvgSave(vg);
//	nvgClearState(vg);

	// Window

	headerPaint = nvgLinearGradient(vg, x,y,x,y+15,
		nvgRGBAf(color.r + 0.5f, color.g + 0.5f, color.b + 0.5f, color.a - 0.3f),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	//nvgFillColor(vg, nvgRGBA(28,30,34,192));
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	//nvgFill(vg);

	//nvgFillColor(vg, nvgRGBA(155,155,155,255));
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	// RAE_TODO nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f * a()));
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	// Drop shadow
	// RAE_TODO shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
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
	// RAE_TODO nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()));
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, x+w/2,y+16+1, title.c_str(), nullptr);

	// Actual title text
	nvgFontBlur(vg,0);
	//textcolor:
	//nvgFillColor(vg, nvgRGBA(220,220,220,160));
	// RAE_TODO nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,a()));
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, x+w/2,y+16, title.c_str(), nullptr);

	nvgRestore(vg);
}

void UISystem::renderButtonNano(NVGcontext* vg, const String& text, float x, float y, float w, float h,
							float cornerRadius, const Color& color, const Color& textColor)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative buttons please:
	if(w < 5.0f) w = 5.0f;
	if(h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, x, y, x, y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

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
	shadowPaint = nvgBoxGradient(vg, x, y+5, w,h, cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x - 60, y - 60, w + 120, h + 120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));
	nvgText(vg, x + w / 2, y + 16 + 1, text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(textColor.r, textColor.g, textColor.b, textColor.a));
	nvgText(vg, x + w / 2, y + 16, text.c_str(), nullptr);

	nvgRestore(vg);
}

Id UISystem::createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));
	addCommand(id, Command(handler));
	return id;
}

Id UISystem::createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));

	// When button gets clicked, change property
	addCommand(id, Command([this, &property]()
	{
		property = !property;
		//rae_log("Set property to: ", Utils::toString(property));
	}));

	bindActive(id, property);

	return id;
}

void UISystem::bindActive(Id id, Bool& property)
{
	// Set current active
	setActive(id, property);

	// When property changes, change active
	property.onChanged.connect(
	[this, id](bool isEnabled)
	{
		//rae_log("Changed to ", Utils::toString(isEnabled), " on: ", id, " : ", getText(id).text);
		setActive(id, isEnabled);
	});
}

Id UISystem::createTextBox(const String& text, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));
	m_transformSystem.setPosition(id, position);

	vec3 halfExtents = extents / 2.0f;
	addBox(id, Box(-(halfExtents), halfExtents));
	addText(id, text);
	return id;
}

Id UISystem::createPanel(const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	addBox(id, Box(-(halfExtents), halfExtents));
	addPanel(id, Panel());
	return id;
}

void UISystem::addPanel(Id id, Panel&& panel)
{
	m_panels.assign(id, std::move(panel));
}

const Panel& UISystem::getPanel(Id id)
{
	return m_panels.get(id);
}

void UISystem::addLayout(Id id)
{
	m_layouts.assign(id, Layout());
}

void UISystem::addToLayout(Id layoutId, Id childId)
{
	if (m_layouts.check(layoutId))
	{
		auto& layout = m_layouts.getF(layoutId);
		if (std::find(layout.children.begin(), layout.children.end(), childId) == layout.children.end())
		{
			layout.children.emplace_back(childId);
		}
	}
}

void UISystem::addBox(Id id, Box&& box)
{
	m_boxes.assign(id, std::move(box));
}

const Box& UISystem::getBox(Id id)
{
	return m_boxes.get(id);
}

void UISystem::addText(Id id, const String& text)
{
	m_texts.assign(id, std::move(Text(text)));
}

void UISystem::addText(Id id, Text&& text)
{
	m_texts.assign(id, std::move(text));
}

const Text& UISystem::getText(Id id)
{
	return m_texts.get(id);
}

void UISystem::addButton(Id id, Button&& element)
{
	m_buttons.assign(id, std::move(element));
}

const Button& UISystem::getButton(Id id)
{
	return m_buttons.get(id);
}

void UISystem::addColor(Id id, Color&& element)
{
	m_colors.assign(id, std::move(element));
}

const Color& UISystem::getColor(Id id)
{
	return m_colors.get(id);
}

void UISystem::addCommand(Id id, Command&& element)
{
	m_commands.assign(id, std::move(element));
}

const Command& UISystem::getCommand(Id id)
{
	return m_commands.get(id);
}

void UISystem::setHovered(Id id, bool hovered)
{
	if (hovered)
		m_hovers.assign(id, std::move(Hover()));
	else m_hovers.remove(id);
}

bool UISystem::isHovered(Id id)
{
	return m_hovers.check(id);
}

void UISystem::setActive(Id id, bool active)
{
	m_actives.assign(id, std::move(Active(active)));
}

bool UISystem::isActive(Id id)
{
	return m_actives.get(id);
}
