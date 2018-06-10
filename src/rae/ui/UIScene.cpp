#include "rae/ui/UISystem.hpp"

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "loguru/loguru.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/visual/Box.hpp"

using namespace rae;

UIScene::UIScene(
	const String& name,
	const Time& time,
	Input& input,
	ScreenSystem& screenSystem,
	DebugSystem& debugSystem) :
		ISystem(name),
		m_entitySystem("UISystem"),
		m_transformSystem(),
		m_selectionSystem(m_transformSystem),
		m_input(input),
		m_screenSystem(screenSystem),
		m_debugSystem(debugSystem)
{
	addTable(m_windows);
	addTable(m_texts);
	addTable(m_buttons);
	addTable(m_commands);
	addTable(m_colors);
	addTable(m_actives);
	addTable(m_viewports);
	addTable(m_panels);
	addTable(m_keylines);
	addTable(m_keylineLinks);
	addTable(m_stackLayouts);
	addTable(m_imageLinks);

	createDefaultTheme();

	m_infoButtonId = createButton("Info",
	vec3(350.0f, 93.0f, 0.0f),
	vec3(69.0f, 6.0f, 1.0f),
	[](){});

	//LOG_F("UISystem creating Info button: %i", m_infoButtonId);
}

void UIScene::createDefaultTheme()
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
	m_panelThemeColors[(size_t)PanelThemeColorKey::Hover]		= Utils::createColor8bit(54, 68, 75, 235);
}

void UIScene::handleInput(const Array<InputEvent>& events)
{
	m_hadEvents = !events.empty();

	// RAE_TODO check 1st mouse button pressed // MouseButton::First
	bool mouseClicked = false;
	for (auto&& event : events)
	{
		if (event.eventType == EventType::MouseButtonRelease)
		{
			mouseClicked = true;
			break;
		}
		else if (event.eventType == EventType::MouseEnter)
		{
			m_mouseInside = true;
		}
		else if (event.eventType == EventType::MouseLeave)
		{
			m_mouseInside = false;
			// Need to clear hovers, so that none are left behind.
			m_selectionSystem.clearHovers();
		}
	}

	if (mouseClicked)
	{
		LOG_F(INFO, "mouseClicked on UIScene name: %s", name().c_str());

		query<Button>(m_buttons, [&](Id id, const Button& button)
		{
			bool hovered = m_selectionSystem.isHovered(id);
			if (hovered && m_commands.check(id))
			{
				auto& command = getCommand(id);
				command.executeAsync();
				//command.execute();
			}
		});
	}
}

UpdateStatus UIScene::update()
{
	static int frameCount = 0;

	m_transformSystem.update(); // RAE_TODO return value.

	doLayout();

	if (m_mouseInside && m_hadEvents)
	{
		hover();
	}

	// Execute async
	{
		query<Command>(m_commands, [&](Id id, Command& command)
		{
			command.update();
		});
	}

	// debug rendering
	if (m_buttons.check(m_infoButtonId))
	{
		auto& button = m_buttons.get(m_infoButtonId);
		const auto& transform = m_transformSystem.getTransform(m_infoButtonId);
		button.setText(
			/*"Mouse:"
			" raw x: " + Utils::toString(m_input.mouse.xP / m_windowSystem.mainWindow().screenPixelRatio())
			+ " raw y: " + Utils::toString(m_input.mouse.yP / m_windowSystem.mainWindow().screenPixelRatio())*/

			" xMM: " + Utils::toString(m_input.mouse.xMM)
			+ " yMM: " + Utils::toString(m_input.mouse.yMM)

			+ " xP: " + Utils::toString(m_input.mouse.xP)
			+ " yP: " + Utils::toString(m_input.mouse.yP)

			+ " x: " + Utils::toString(m_input.mouse.x)
			+ " y: " + Utils::toString(m_input.mouse.y)
			//+ " frame: " + Utils::toString(frameCount)
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
			//LOG_F(INFO, "UISystem::render settings stuff mouse.x: %f mouse.y: %f",
			//	m_input.mouse.x, m_input.mouse.y);

			transform.setTarget(vec3(m_input.mouse.x, m_input.mouse.y, 0.0f), 1.0f);
			//LOG_F(INFO, "Click %i", frameCount);
		}
	}
	*/

	frameCount++;

	return UpdateStatus::NotChanged;
}

void UIScene::doLayout()
{
	query<StackLayout>(m_stackLayouts, [&](Id layoutId, StackLayout& layout)
	{
		if (m_transformSystem.hasChildren(layoutId))
		{
			auto& children = m_transformSystem.getChildren(layoutId);
			//Array<Id> children(childrenSet.begin(), childrenSet.end());
			//layout.doLayout(children);

			const vec3& parentPos = m_transformSystem.getPosition(layoutId);
			const Pivot& parentPivot = m_transformSystem.getPivot(layoutId);
			Box parentBox = m_transformSystem.getBox(layoutId);
			parentBox.translate(parentPivot);

			// RAE_TODO Some kind of margin: float marginMM = 6.0f;
			float someIter = parentPos.y + parentBox.min().y;
			for (auto&& childId : children)
			{
				vec3 pos = m_transformSystem.getPosition(childId);
				const Pivot& pivot = m_transformSystem.getPivot(childId);
				Box tbox = m_transformSystem.getBox(childId);
				tbox.translate(pivot);

				pos.x = (parentPos.x + parentBox.min().x) - tbox.min().x;// + marginMM;
				pos.y = someIter - tbox.min().y;// + marginMM;
				m_transformSystem.setPosition(childId, pos);
				someIter = someIter + tbox.dimensions().y;
			}
		}

		//RAE_TODO use owner owned? or layoutParent layoutChildren?
		// or some other type of additional hierarchy, so that layout could even be a member of the parent panel here
		// and it would do layout on its siblings.
	});

	query<KeylineLink>(m_keylineLinks, [&](Id id, const KeylineLink& keylineLink)
	{
		if (m_keylines.check(keylineLink.keylineId))
		{
			auto& keyline = getKeyline(keylineLink.keylineId);
			vec3 origPos = m_transformSystem.getPosition(id);

			float posX = m_transformSystem.getBox(m_rootId).dimensions().x * keyline.relativePosition;
			m_transformSystem.setPosition(id, vec3(posX, origPos.y, origPos.z));
		}
	});
}

void UIScene::hover()
{
	// hover boxes
	for (Id id : m_entitySystem.entities())
	{
		if (m_transformSystem.hasTransform(id)
			&& m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);
			Box tbox = m_transformSystem.getBox(id);
			tbox.transform(transform);
			tbox.translate(pivot);

			if (tbox.hit(vec2(m_input.mouse.xMM, m_input.mouse.yMM)))
			{
				m_selectionSystem.setHovered(id, true);
			}
			else
			{
				m_selectionSystem.setHovered(id, false);
			}
		}
	}
}

void UIScene::render2D(NVGcontext* nanoVG, const AssetSystem& assetSystem)
{
	/*
	RAE_REMOVE
	const auto& window = m_windowSystem.window();

	int windowWidth = window.width();
	int windowHeight = window.height();
	float screenPixelRatio = window.screenPixelRatio();
	*/

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

	if (m_debugSystem.isEnabled())
	{
		// Debug rendering border for WindowEntities.
		query<WindowEntity>(m_windows, [&](Id id)
		{
			if (m_transformSystem.hasTransform(id) and
				m_transformSystem.hasBox(id))
			{
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = m_transformSystem.getBox(id);
				const Pivot& pivot = m_transformSystem.getPivot(id);

				bool hasColor = m_colors.check(id);
				bool hovered = m_selectionSystem.isHovered(id);

				float cornerRadius = 0.0f;
				float thicknessMM = 5.0f;
				renderBorder(transform, box, pivot,
					hovered ? Colors::magenta * 1.5f :
					hasColor ? getColor(id) :
					Colors::magenta * 0.5f,
					cornerRadius, thicknessMM);
			}
		});
	}

	query<Viewport>(m_viewports, [&](Id id, const Viewport& viewport)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool hovered = m_selectionSystem.isHovered(id);

			renderBorder(transform, box, pivot,
				hovered ? panelHoverColor :
				hasColor ? getColor(id) :
				panelBackgroundColor);
		}
	});

	query<Panel>(m_panels, [&](Id id, const Panel& panel)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool hovered = m_selectionSystem.isHovered(id);

			renderRectangle(transform, box, pivot,
					hovered ? panelHoverColor :
					hasColor ? getColor(id) :
					panelBackgroundColor);
		}
	});

	query<Button>(m_buttons, [&](Id id, const Button& button)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool active = isActive(id);
			bool hovered = m_selectionSystem.isHovered(id);

			renderButton(button.text(), transform, box, pivot,
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
	});

	query<ImageLink>(m_imageLinks, [&](Id id, const ImageLink& imageLink)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			renderImage(imageLink, transform, box, pivot, assetSystem);
		}
	});

	if (m_debugSystem.isEnabled())
	{
		query<Keyline>(m_keylines, [&](Id id, const Keyline& keyline)
		{
			float fromX = m_screenSystem.mmToPixels(
				m_transformSystem.getBox(
					m_rootId).dimensions().x * keyline.relativePosition);
			float toX = fromX;
			float fromY = 0.0f;
			float toY = 800.0f;

			renderLineNano(m_nanoVG, vec2(fromX, fromY), vec2(toX, toY), Colors::orange);
		});

		query<Transform>(m_transformSystem.transforms(), [&](Id id, const Transform& transform)
		{
			float diameter = 2.0f;
			renderCircle(transform, diameter, Colors::cyan);
		});

		if (m_mouseInside)
		{
			// Render mouse cursor as a circle (actually with arc function)
			renderArc(
				vec2(m_input.mouse.xMM, m_input.mouse.yMM),
				0.0f,				// fromAngleRad
				Math::TAU * 1.0f,	// toAngleRad
				20.0f,				// diameter
				1.0f,				// thickness
				m_input.mouse.button(MouseButton::First) ?
				Colors::red :
				Colors::lightGray * Color(1.0f, 1.0f, 1.0f, 0.5f));
		}
	}
}

Rectangle UIScene::convertToRectangle(const Transform& transform, const Box& box, const Pivot& pivot) const
{
	vec3 dimensions = box.dimensions();
	Box pivotedBox = box;
	pivotedBox.translate(pivot);

	return Rectangle(
		m_screenSystem.mmToPixels(transform.position.x + pivotedBox.left()),
		m_screenSystem.mmToPixels(transform.position.y + pivotedBox.down()), // Aaargh, Y down vs Y up issue. That's why this is down. Or should it be up?
		m_screenSystem.mmToPixels(dimensions.x),
		m_screenSystem.mmToPixels(dimensions.y));
}

void UIScene::renderBorder(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color,
	float cornerRadius, float thickness)
{
	renderBorderNano(m_nanoVG,
		convertToRectangle(transform, box, pivot),
		color,
		m_screenSystem.mmToPixels(cornerRadius),
		thickness == 0.0f ? 1.0f : m_screenSystem.mmToPixels(thickness));
}

void UIScene::renderCircle(const Transform& transform, float diameter, const Color& color)
{
	renderCircle(vec2(transform.position.x, transform.position.y), diameter, color);
}

void UIScene::renderCircle(const vec2& position, float diameter, const Color& color)
{
	renderCircleNano(m_nanoVG,
		m_screenSystem.mmToPixels(position),
		m_screenSystem.mmToPixels(diameter),
		color);
}

void UIScene::renderArc(const vec2& origin, float fromAngleRad, float toAngleRad,
	float diameter, float thickness, const Color& color)
{
	renderArcNano(m_nanoVG, m_screenSystem.mmToPixels(origin), fromAngleRad, toAngleRad,
		m_screenSystem.mmToPixels(diameter), m_screenSystem.mmToPixels(thickness), color);
}

void UIScene::renderRectangle(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color)
{
	renderRectangleNano(m_nanoVG,
		convertToRectangle(transform, box, pivot),
		0.0f, // cornerRadius
		color);
}

void UIScene::renderButton(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
	const Color& color, const Color& textColor)
{
	renderButtonNano(m_nanoVG, text,
		convertToRectangle(transform, box, pivot),
		m_screenSystem.mmToPixels(0.46f), // cornerRadius
		color,
		textColor);
}

void UIScene::renderImage(ImageLink imageLink, const Transform& transform, const Box& box, const Pivot& pivot,
const AssetSystem& assetSystem)
{
	const auto& image = assetSystem.getImage(imageLink);

	auto rect = convertToRectangle(transform, box, pivot);

	renderImageNano(m_nanoVG, image.imageId(), rect.x, rect.y, rect.width, rect.height);
}

void UIScene::renderLineNano(NVGcontext* vg, const vec2& from, const vec2& to,
			const Color& color)
{
	nvgSave(vg);

	NVGcolor strokeColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgStrokeColor(vg, strokeColor);
	nvgStrokeWidth(vg, 1.0f);

	nvgMoveTo(vg, from.x, from.y);
	nvgLineTo(vg, to.x, to.y);
	nvgStroke(vg);

	nvgRestore(vg);
}

void UIScene::renderCircleNano(NVGcontext* vg, const vec2& position,
	float diameter, const Color& color)
{
	nvgSave(vg);

	NVGcolor fillColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgCircle(vg, position.x, position.y, diameter * 0.5f);
	nvgFillColor(vg, fillColor);
	nvgFill(vg);
	//nvgStrokeColor(vg, strokeColor);
	//nvgStrokeWidth(vg, 1.0f);
	//nvgStroke(vg);

	nvgRestore(vg);
}

void UIScene::renderArcNano(NVGcontext* vg, const vec2& origin, float fromAngleRad, float toAngleRad,
	float diameter, float thickness, const Color& color)
{
	NVGcolor fillColor = nvgRGBAf(color.r, color.g, color.b, color.a);
	//NVGcolor strokeColor = nvgRGBAf(0.0f, 0.0f, 0.0f, color.a);

	vec2 arcVec1 = vec2(cos(fromAngleRad), sin(fromAngleRad));
	vec2 arcVec2 = vec2(cos(toAngleRad), sin(toAngleRad));

	vec2 radii = vec2((diameter * 0.5f) - thickness, diameter * 0.5f);

	vec2 pos1 = origin + (arcVec1 * radii.x);
	vec2 pos2 = origin + (arcVec1 * radii.y);

	vec2 pos3 = origin + (arcVec2 * radii.x);
	vec2 pos4 = origin + (arcVec2 * radii.y);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgMoveTo(vg, pos1.x, pos1.y);
	nvgLineTo(vg, pos2.x, pos2.y);
	nvgArc(vg, origin.x, origin.y, radii.x, fromAngleRad, toAngleRad, NVG_CW);
	nvgLineTo(vg, pos3.x, pos3.y);
	nvgArc(vg, origin.x, origin.y, radii.y, toAngleRad, fromAngleRad, NVG_CCW);
	nvgClosePath(vg);

	//nvgStrokeColor(vg, strokeColor);
	//nvgStroke(vg);

	nvgFillColor(vg, fillColor);
	nvgFill(vg);

	nvgRestore(vg);
}

void UIScene::renderBorderNano(
	NVGcontext* vg,
	const Rectangle& rectangle,
	const Color& color,
	float cornerRadius,
	float thickness)
{
	// No negatives please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	NVGcolor strokeColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgStrokeColor(vg, strokeColor);
	nvgStrokeWidth(vg, thickness);
	nvgStroke(vg);

	nvgRestore(vg);
}

void UIScene::renderRectangleNano(NVGcontext* vg, const Rectangle& rectangle,
	float cornerRadius, const Color& color) const
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negatives please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f),
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x - 60, rectangle.y - 60, rectangle.width + 120, rectangle.height + 120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void UIScene::renderWindowNano(NVGcontext* vg, const String& title, const Rectangle& rectangle,
							float cornerRadius, const Color& color)
{
	//float cornerRadius = 30.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative windows please:
	/*
	if (w < 30.0f)
	{
		w = 30.0f;
	}

	if (h < 30.0f)
	{
		h = 30.0f;
	}
	*/

	nvgSave(vg);
//	nvgClearState(vg);

	// Window

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y+15,
		nvgRGBAf(color.r + 0.5f, color.g + 0.5f, color.b + 0.5f, color.a - 0.3f),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
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
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x-60, rectangle.y-60, rectangle.width+120, rectangle.height+120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
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
	nvgText(vg, rectangle.x + rectangle.width/2, rectangle.y+16+1, title.c_str(), nullptr);

	// Actual title text
	nvgFontBlur(vg,0);
	//textcolor:
	//nvgFillColor(vg, nvgRGBA(220,220,220,160));
	// RAE_TODO nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,a()));
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, rectangle.x + rectangle.width/2, rectangle.y+16, title.c_str(), nullptr);

	nvgRestore(vg);
}

void UIScene::renderButtonNano(NVGcontext* vg, const String& text, const Rectangle& rectangle,
							float cornerRadius, const Color& color, const Color& textColor)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative buttons please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	/*
	// Outline
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);
	*/

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x - 60, rectangle.y - 60, rectangle.width + 120, rectangle.height + 120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));

	const float shadowOffset = 1.0f;
	const float rectangleHalfWidth = rectangle.width * 0.5f;
	const float rectangleHalfHeight = rectangle.height * 0.5f;

	nvgText(vg, rectangle.x + rectangleHalfWidth, rectangle.y + rectangleHalfHeight + shadowOffset, text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(textColor.r, textColor.g, textColor.b, textColor.a));
	nvgText(vg, rectangle.x + rectangleHalfWidth, rectangle.y + rectangleHalfHeight, text.c_str(), nullptr);

	nvgRestore(vg);
}

Id UIScene::connectToWindow(const Window& window)
{
	Id id = m_entitySystem.createEntity();
	m_rootId = id;
	m_windows.assign(id, WindowEntity());

	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	m_transformSystem.addTransform(id, Transform(position));
	updateWindowSize(window);
	m_transformSystem.addPivot(id, Pivot(-1.0f, -1.0f, 0.0f));

	return id;
}

void UIScene::updateWindowSize(const Window& window)
{
	vec3 halfExtents = m_screenSystem.pixelsToMM(vec3(window.width(), window.height(), 1.0f) / 2.0f);
	m_transformSystem.addBox(m_rootId, Box(-(halfExtents), halfExtents));
}

Id UIScene::createButton(const String& text, std::function<void()> handler)
{
	return createButton(text, vec3(), vec3(32.0f, 16.0f, 1.0f), handler);
}

Id UIScene::createButton(const String& text, const Rectangle& rectangle, std::function<void()> handler)
{
	Id button = createButton(text, vec3(rectangle.x, rectangle.y, 0.0f), vec3(rectangle.width, rectangle.height, 1.0f),
		handler);
	m_transformSystem.addPivot(button, Pivots::TopLeft2D);
	return button;
}

Id UIScene::createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));
	addCommand(id, Command(handler));
	return id;
}

Id UIScene::createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));

	// When button gets clicked, change property
	addCommand(id, Command([this, &property]()
	{
		property = !property;
		//LOG_F(INFO, "Set property to: %s", Utils::toString(property));
	}));

	bindActive(id, property);

	return id;
}

void UIScene::bindActive(Id id, Bool& property)
{
	// Set current active
	setActive(id, property);

	// When property changes, change active
	property.onChanged.connect(
	[this, id](bool isEnabled)
	{
		//LOG_F(INFO, "Changed to %s on: %i : %s", Utils::toString(isEnabled), id, getText(id).text);
		setActive(id, isEnabled);
	});
}

Id UIScene::createTextBox(const String& text, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));
	m_transformSystem.setPosition(id, position);

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addText(id, text);
	return id;
}

Id UIScene::createViewport(int sceneIndex, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addViewport(id, Viewport(sceneIndex));
	return id;
}

void UIScene::addViewport(Id id, Viewport&& entity)
{
	m_viewports.assign(id, std::move(entity));
}

const Viewport& UIScene::getViewport(Id id)
{
	return m_viewports.get(id);
}

Rectangle UIScene::getViewportPixelRectangle(int sceneIndex) const
{
	Rectangle viewportRect;
	query<Viewport>(m_viewports, [&](Id id, const Viewport& viewport)
	{
		if (viewport.sceneIndex == sceneIndex and
			m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			viewportRect = convertToRectangle(transform, box, Pivots::Center);
		}
	});
	return viewportRect;
}

Id UIScene::createPanel(const Rectangle& rectangle)
{
	Id entity = createPanel(vec3(rectangle.x, rectangle.y, 0.0f), vec3(rectangle.width, rectangle.height, 1.0f));
	m_transformSystem.addPivot(entity, Pivots::TopLeft2D);
	return entity;
}

Id UIScene::createPanel(const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addPanel(id, Panel());
	return id;
}

void UIScene::addPanel(Id id, Panel&& panel)
{
	m_panels.assign(id, std::move(panel));
}

const Panel& UIScene::getPanel(Id id)
{
	return m_panels.get(id);
}

void UIScene::addStackLayout(Id id)
{
	m_stackLayouts.assign(id, StackLayout());
}

/*RAE_TODO owner or layoutChildren thing
void UIScene::addToLayout(Id layoutId, Id childId)
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
*/

Id UIScene::createImageBox(asset::Id imageLink, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addImageLink(id, ImageLink(imageLink));
	return id;
}

void UIScene::addImageLink(Id id, ImageLink entity)
{
	m_imageLinks.assign(id, std::move(entity));
}

const ImageLink& UIScene::getImageLink(Id id)
{
	return m_imageLinks.get(id);
}

Id UIScene::createKeyline(Keyline&& element)
{
	Id id = m_entitySystem.createEntity();
	addKeyline(id, std::move(element));
	return id;
}

void UIScene::addKeyline(Id id, Keyline&& element)
{
	m_keylines.assign(id, std::move(element));
}

const Keyline& UIScene::getKeyline(Id id)
{
	return m_keylines.get(id);
}

void UIScene::addKeylineLink(Id childId, Id keylineId) //TODO anchor
{
	m_keylineLinks.assign(childId, KeylineLink(keylineId));
}

const KeylineLink& UIScene::getKeylineLink(Id id)
{
	return m_keylineLinks.get(id);
}

void UIScene::addText(Id id, const String& text)
{
	m_texts.assign(id, std::move(Text(text)));
}

void UIScene::addText(Id id, Text&& text)
{
	m_texts.assign(id, std::move(text));
}

const Text& UIScene::getText(Id id)
{
	return m_texts.get(id);
}

void UIScene::addButton(Id id, Button&& element)
{
	m_buttons.assign(id, std::move(element));
}

const Button& UIScene::getButton(Id id)
{
	return m_buttons.get(id);
}

void UIScene::addColor(Id id, Color&& element)
{
	m_colors.assign(id, std::move(element));
}

const Color& UIScene::getColor(Id id)
{
	return m_colors.get(id);
}

void UIScene::addCommand(Id id, Command&& element)
{
	m_commands.assign(id, std::move(element));
}

Command& UIScene::getCommand(Id id)
{
	return m_commands.get(id);
}

const Command& UIScene::getCommand(Id id) const
{
	return m_commands.get(id);
}

void UIScene::setActive(Id id, bool active)
{
	m_actives.assign(id, std::move(Active(active)));
}

bool UIScene::isActive(Id id)
{
	return m_actives.get(id);
}
